#include "unit_test.hh"

BasicPageMappingTestFixture::BasicPageMappingTestFixture(){
  conf = nullptr;
  p_ftl = nullptr;
  p_pmap = nullptr;
  p_dram = nullptr;
  cfg_info = nullptr;
}

void BasicPageMappingTestFixture::SetUp(){
  // called before every test
  remakeFTL(conf, p_ftl, p_pmap, p_dram, cfg_info);
  p_pmap->resetStatValues();
}

void BasicPageMappingTestFixture::TearDown(){
  // called after every test;
  clear_ptr(conf);
  clear_ptr(p_ftl);
  clear_ptr(p_dram);
  clear_ptr(cfg_info);
}

void PageMappingTestFixture::SetUp(){
  // called before every test
}

void PageMappingTestFixture::init(){
  remakeFTL(conf, p_ftl, p_pmap, p_dram, cfg_info);
  p_pmap->resetStatValues();
}

void PageMappingTestFixture::TearDown(){
  //called after every test
}

void PageMappingTestFixture::clear(){
  clear_ptr(conf);
  clear_ptr(p_ftl);
  clear_ptr(p_dram);
  clear_ptr(cfg_info);
}

void PageMappingTestFixture::reset(){
  clear();
  init();
}

void PageMappingTestFixture::GCOrdinaryTest(uint32_t write_pages){
  //remake FTL;
  reset();
  Request req = Request(ioUnitInPage);
  req.ioFlag.set();
  uint64_t tick = 0LL;
  for(uint64_t i = 0; i<write_pages; ++i){
    req.lpn = i;
    p_pmap->write(req, tick);
    if(i % (write_pages / 4) == 0){
      std::cout <<"OverWriteTest-Finished pages: " << i << "/" << write_pages << std::endl;
    }
  }
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(block_stat.compressUnitCount, 0);
  EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
  EXPECT_EQ(p_pmap->stat.decompressCount, 0);
  EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
  EXPECT_GT(p_pmap->stat.gcCount, 10);
  EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, 0);
  EXPECT_GT(p_pmap->stat.reclaimedBlocks, 90);
  EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
  EXPECT_GE(p_pmap->stat.totalWriteIoUnitCount, ioUnitInPage * write_pages);
  printFTLInfo(p_pmap, block_stat, "unit_test_none_P" + std::to_string(write_pages));
}

void PageMappingTestFixture::GCCompressTest(UTTestInfo& test_info){
  //remake FTL;
  reset();
  Request req = Request(ioUnitInPage);
  req.cd_info.offset = 0;
  req.cd_info.pDisk = createTestDisk(test_info.comptype, test_info.dipolicy,cfg_info->nPagesToWarmup);
  EXPECT_GE(((SimpleSSD::CompressedDisk*)(req.cd_info.pDisk))->compress_unit_totalcnt, cfg_info->nPagesToWarmup * ioUnitInPage);
  req.ioFlag.set();
  uint64_t tick = 0LL;
  ASSERT_TRUE(ioUnitInPage * ioUnitSize == 65536);
  uint8_t buffer[65536]; // 64KiB
  memset(buffer, 0, 65536);
  srand(RANDOM_SEED);
  for(uint64_t i = 0; i<test_info.write_pages; ++i){
    req.lpn = i;
    p_pmap->write(req, tick);
    //Sync to Disk
    if(test_info.dwpolicy == DiskWritePolicy::BYTE_RANDOM){
      for(uint32_t i = 0; i<65536; ++i){
        buffer[i] = getRandomByte();
      }
    }
    else if(test_info.dwpolicy == DiskWritePolicy::ZERO){
      //Do Nothing.
    }
    else if(test_info.dwpolicy == DiskWritePolicy::CUSTOM){
      memcpy(buffer, test_info.write_data, 65536);
    }
    else{
      assert(false && "Not support this disk write policy.");
    }
    req.cd_info.pDisk->writeOrdinary(req.lpn*65536, 65536, buffer);
    if(i % (test_info.write_pages / 4) == 0){
      std::cout <<"gcTest-Finished pages: " << i << "/" << test_info.write_pages << std::endl;
    }
  }
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(p_pmap->stat.decompressCount, 0);
  EXPECT_GE(p_pmap->stat.failedCompressCout, 0);
  EXPECT_GT(p_pmap->stat.gcCount, 10);
  EXPECT_GE(p_pmap->stat.overwriteCompressUnitCount, 0);
  EXPECT_GT(p_pmap->stat.reclaimedBlocks, 90);
  EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
  EXPECT_GE(p_pmap->stat.totalWriteIoUnitCount, ioUnitInPage * test_info.write_pages);
  if(test_info.dwpolicy == DiskWritePolicy::ZERO || test_info.dwpolicy == DiskWritePolicy::CUSTOM){
    EXPECT_GT(block_stat.compressUnitCount, 0);
  }
  else if(test_info.dwpolicy == DiskWritePolicy::BYTE_RANDOM){
    EXPECT_EQ(block_stat.compressUnitCount, 0);
  }
  else{
    assert(false && "Not support yet.");
  }
  EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
  EXPECT_LE(block_stat.validDataLength, block_stat.totalDataLength);
  EXPECT_LE(block_stat.validIoUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
  // print infos;
  printFTLInfo(p_pmap, block_stat, test_info.getTestName());
  delete req.cd_info.pDisk;
}


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

//undef region
#undef clear_ptr
#undef is_ratio
#undef get_percent
