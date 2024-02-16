#include "unit_test.hh"

/**
  *implementation of help functions.
**/
void remakeFTL(SimpleSSD::ConfigReader* &conf, FTL* &p_ftl, PageMapping* &p_pmap, SimpleSSD::DRAM::AbstractDRAM* &p_dram, ConfigInfo* &cfg_info){
  clear_ptr(conf);
  clear_ptr(p_ftl);
  clear_ptr(p_dram);
  clear_ptr(cfg_info);
  conf = new SimpleSSD::ConfigReader();
  const string* s = &simple_config_file;
  if(test_cfg==TestConfig::SIMPLE){
      s = &simple_config_file;
      pageCount = 16;
  }
  else if(test_cfg == TestConfig::ACTUAL){
      s = &actual_config_file;
      pageCount = 768;
  }
  else{
      assert(0 && "No such TestConfig.");
  }
  if(!conf->init(*s)){
      assert(0 && "Failed to laod file.");
  }
  p_dram = new SimpleSSD::DRAM::SimpleDRAM(*conf);
  p_ftl = new FTL(*conf, p_dram);
  p_pmap = (PageMapping*)p_ftl->pFTL;
  cfg_info = new ConfigInfo();
  cfg_info->nTotalLogicalPages = p_pmap->param.totalLogicalBlocks * p_pmap->param.pagesInBlock;
  cfg_info->nPagesToWarmup = cfg_info->nTotalLogicalPages * p_pmap->conf.readFloat(SimpleSSD::CONFIG_FTL, FTL_FILL_RATIO);
  cfg_info->nPagesToInvalidate = cfg_info->nTotalLogicalPages * p_pmap->conf.readFloat(SimpleSSD::CONFIG_FTL, FTL_INVALID_PAGE_RATIO);
}

void printFTLInfo(PageMapping* p_pmap,const BlockStat& block_stat, const std::string &test_name) {
  std::vector<SimpleSSD::Stats> stats;
  std::vector<double> values;
  std::string prefix = "system.pc.ftl.page_mapping";
  p_pmap->getStatList(stats, prefix);
  p_pmap->getStatValues(values);
  ASSERT_EQ(values.size(), stats.size());
  utStatFile << "--------------------------------------" << std::endl;
  utStatFile<<test_name<<" | "<< pageCount << " | ";;
  outputUTStats(block_stat, p_pmap);
  printstat_ftlcnt++;
  utStatFile << "This is the " << printstat_ftlcnt << "-th output of FTL statistics data." << std::endl;
  for(size_t i = 0; i<values.size(); ++i){
    std::string out = "";
    out+= stats[i].name;
    out+= "           ";
    out+= std::to_string((long long)values[i]);
    out+= "           //";
    out+= stats[i].desc;
    utStatFile << out << std::endl;
  }
  utStatFile << "--------------------------------------" << std::endl;
}

bool pyRun(const std::string file_name, const std::string input, std::string& output){
  //make command.
  std::string command = "echo \'" + input + "\' | python3 " + file_name;
  FILE* pipe = popen(command.c_str(), "r");
  if(!pipe){
    std::cout << "Open python file failed, errono is " << errno << "." << std::endl;
    return false;
  }
  output.clear();
  char buffer[128];
  while(fgets(buffer, 128, pipe) != nullptr){
    output += buffer;
  }
  pclose(pipe);
  return true;
}

bool parseStatAnalyzerOut(const std::string& output, StatAnalyzerOut& out){
  std::vector<double> numbers;
  //use regex to parse 
  std::regex floatRegex(R"([-+]?[0-9]*\.?[0-9]+([eE][-+]?[0-9]+)?)");
  auto words_begin = std::sregex_iterator(output.begin(), output.end(), floatRegex);
  auto words_end = std::sregex_iterator();
  // 遍历匹配结果，输出浮点数
  for (std::sregex_iterator i = words_begin; i != words_end; ++i) {
    std::smatch match = *i;
    std::string match_str = match.str();
    numbers.push_back(stod(match_str));
  }

  //check valid
  if(numbers.size() != 3){
    return false;
  }
  out.r_c = numbers[0];
  out.f_c = numbers[1];
  out.r_f = numbers[2];
  if(!is_ratio(out.r_c) || !is_ratio(out.f_c) || !is_ratio(out.r_f)){
    return false;
  }
  return true;
}

SimpleSSD::Disk* createTestDisk(SimpleSSD::CompressType compress_type, DiskInitPolicy disk_init_policy, uint64_t superpage_cnt){
  if(compress_type == SimpleSSD::CompressType::NONE){
    assert(false && "Not support this type disk.");
  }
  const char* filename = "nvme.img";
  if(access(filename, F_OK) != -1){
    //remove file to make sure all '/0'
    assert(remove(filename) == 0);
  }
  SimpleSSD::Disk* ret =  new SimpleSSD::CompressedDisk();
  ret->open(img_file, superpage_cnt * ioUnitInPage* ioUnitSize, ioUnitSize);
  ((SimpleSSD::CompressedDisk*)(ret))->init(ioUnitSize, compress_type);
  //all zero;
  if(disk_init_policy == DiskInitPolicy::BYTE_RANDOM){
    uint8_t buffer[1];
    srand(RANDOM_SEED);// Seet seed
    for(uint64_t offset = 0; offset<ret->diskSize; offset+=8){
      buffer[0] = getRandomByte();
      assert(ret->writeOrdinary(offset, 8, buffer) == 8);
    }
  }
  else if(disk_init_policy == DiskInitPolicy::ALL_ZERO){
    //do Nothing.
  }
  else{
    assert(false && "Non't suppose this disk init policy.");
  }
  return ret;
}

void outputUTStats(const BlockStat& block_stat, PageMapping* p_pmap){
  //calculate   
  StatAnalyzerOut outstats;
  std::string input = "";
  input += std::to_string(block_stat.totalDataLength) + "\n";
  input += std::to_string(block_stat.validDataLength) + "\n";
  input += std::to_string(block_stat.validIoUnitCount) + "\n";
  input += std::to_string(block_stat.compressUnitCount) + "\n";
  input += std::to_string(block_stat.totalUnitCount) + "\n";
  std::string output = "";
  std::string file_name = "statanalyzer.py";
  assert(pyRun(file_name, input, output));
  assert(parseStatAnalyzerOut(output, outstats));
  //outputFile
  std::string out = "| ";
  out += std::to_string(p_pmap->stat.gcCount);
  out += " | ";
  if(p_pmap->stat.totalReadIoUnitCount == 0){
    out += "-";
  }
  else{
    out += std::to_string((double)(p_pmap->stat.decompressCount) * 100.0D / (double)(p_pmap->stat.totalReadIoUnitCount)) + "%";
  }
  out += " | ";
  if(p_pmap->stat.totalWriteIoUnitCount == 0){
    out += "-";
  }
  else{
    out += std::to_string((double)(p_pmap->stat.overwriteCompressUnitCount) * 100.0D / (double)(p_pmap->stat.totalWriteIoUnitCount)) + "%";
  }
  out += " | ";
  out += std::to_string(block_stat.totalDataLength);
  out += " | ";
  out += std::to_string(block_stat.validDataLength);
  out += " | ";
  out += std::to_string(block_stat.totalUnitCount);
  out += " | ";
  out += std::to_string(block_stat.validIoUnitCount);
  out += " | ";
  out += std::to_string(block_stat.compressUnitCount);
  out += " | ";
  out += std::to_string(outstats.r_c * 100.0D) + "%";
  out += " | ";
  out += std::to_string(outstats.f_c * 100.0D) + "%";
  out += " | ";
  out += std::to_string(outstats.r_f * 100.0D) + "%";
  utStatFile << out << std::endl;
}

void UTTestIterator::init(uint32_t all_pages){
  all_comptype[0] = SimpleSSD::CompressType::LZ4;
  all_comptype[1] = SimpleSSD::CompressType::LZMA;
  all_dipolicy[0] = DiskInitPolicy::ALL_ZERO;
  all_dipolicy[1] = DiskInitPolicy::BYTE_RANDOM;
  all_dwpolicy[0] = DiskWritePolicy::ZERO;
  all_dwpolicy[1] = DiskWritePolicy::BYTE_RANDOM;
  all_writepages.clear();
  for(uint32_t i = 1; i<= wrpage_split_factor; ++i){
    all_writepages.push_back(all_pages * i / (1.0D * wrpage_split_factor));
  }
  iter_num = 0;
}

UTTestInfo UTTestIterator::getnextTestInfo(){
  UTTestInfo ret;
  uint32_t cp_itnum = iter_num;
  ret.write_pages = all_writepages[iter_num % all_writepages.size()];
  iter_num /= all_writepages.size();
  ret.dwpolicy = all_dwpolicy[iter_num % 2];
  iter_num /= 2;
  ret.dipolicy = all_dipolicy[iter_num % 2];
  iter_num /= 2;
  ret.comptype = all_comptype[iter_num % 2];
  iter_num = cp_itnum + 1;
  return ret;
}

bool UTTestIterator::is_end(){
  return iter_num == 8 * all_writepages.size();
}

std::string UTTestInfo::getTestName(){
  std::string ret = "unit_test_";
  if(comptype == SimpleSSD::CompressType::LZ4){
    ret += "lz4_";
  }
  else if(comptype == SimpleSSD::CompressType::LZMA){
    ret += "lzma_";
  }
  else {
    assert(false && "Not support.");
  }
  if(dipolicy == DiskInitPolicy::ALL_ZERO){
    ret += "Izero_";
  }
  else if(dipolicy == DiskInitPolicy::BYTE_RANDOM){
    ret += "Irandom_";
  }
  else{
    assert(false && "Not support.");
  }
  if(dwpolicy == DiskWritePolicy::ZERO){
    ret += "Wzero_";
  }
  else if(dwpolicy == DiskWritePolicy::BYTE_RANDOM){
    ret += "Wrandom_";
  }
  else{
    assert(false && "Not support.");
  }
  ret += "P" + std::to_string(write_pages);
  return ret;
}

//undef region
#undef clear_ptr
#undef is_ratio

int main(int argc, char **argv) {
    // 初始化 Google Test
    ::testing::InitGoogleTest(&argc, argv);

    std::string s;
    // 解析自定义参数
    for (int i = 1; i < argc; ++i) {
        if (std::string(argv[i]) == "--testconfig" && i + 1 < argc) {
            s = argv[i + 1];
            if(s == "simple"){
              test_cfg = TestConfig::SIMPLE;
            }
            else if(s == "actual"){
              test_cfg = TestConfig::ACTUAL;
            }
            else{
              assert(0 && "No such test config options");
            }
            break;
        }
    }

    // 运行测试
    int test_ret =  RUN_ALL_TESTS();
    //do some clear jobs.
    utStatFile.close();
    return test_ret;
}