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

void printFTLInfo(PageMapping* p_pmap){
  std::vector<SimpleSSD::Stats> stats;
  std::vector<double> values;
  std::string prefix = "system.pc.ftl.page_mapping";
  p_pmap->getStatList(stats, prefix);
  p_pmap->getStatValues(values);
  ASSERT_EQ(values.size(), stats.size());
  utStatFile << "--------------------------------------" << std::endl;
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