//author: ZeitHaum
#include "ftl/page_mapping.hh"
#include "dram/simple.hh"
#include "gtest/gtest.h"
#include "random_generator.cc"
#include <thread>
#include <fstream>
#include <memory>
#include <errno.h>
#include <regex>

using namespace SimpleSSD::FTL;

//define region
#define clear_ptr(ptr) if(ptr){ \
  delete ptr; \
  ptr = nullptr;\
} 
#define is_ratio(x) ((x) >= 0 && (x) <= 1)

// global variables' defination
const std::string actual_config_file = "./simplessd/config/samsung_983dct_1.92tb.cfg";
const std::string simple_config_file = "./unit_test.cfg";
const std::string img_file = "./nvme.img";
uint64_t ioUnitInPage = 16;
uint64_t pageCount = 768;
uint64_t ioUnitSize = 4096;
enum class TestConfig{
    SIMPLE, ACTUAL
};
TestConfig test_cfg = TestConfig::SIMPLE;
std::ofstream utStatFile("utstat.txt"); // output ut stats.
uint64_t printstat_ftlcnt = 0;

//declaration of neccessary structures.
struct ConfigInfo{
  uint64_t nTotalLogicalPages;
  uint64_t nPagesToWarmup;
  uint64_t nPagesToInvalidate;
};

struct StatAnalyzerOut{
  double r_c;
  double f_c;
  double r_f;
};

//declaration of neccessary help functions.
void remakeFTL(SimpleSSD::ConfigReader* &conf, FTL* &p_ftl, PageMapping* &p_pmap, SimpleSSD::DRAM::AbstractDRAM* &p_dram, ConfigInfo* &cfg_info);

void printFTLInfo(PageMapping* p_pmap);

bool pyRun(const std::string file_name, const std::string input, std::string& output);

bool parseStatAnalyzerOut(const std::string& output, StatAnalyzerOut& out);

SimpleSSD::Disk* createTestDisk(SimpleSSD::CompressType compress_type, uint64_t superpage_cnt);

void outputUTStats(const BlockStat& block_stat, PageMapping* p_pmap);

//Unit_test of help functions;
TEST(PyRunTest, statAnalyzerTest){
  std::string file_name = "statanalyzer.py";
  std::string output = "";
  //Random generate input
  std::string test_input = "42501144576\n42097716496\n10289809\n99440\n10376256\n";
  ASSERT_TRUE(pyRun(file_name, test_input, output));
  StatAnalyzerOut out;
  ASSERT_TRUE(parseStatAnalyzerOut(output, out));
  EXPECT_LE(abs(out.r_c - 0.0095D), 0.0001D);
  EXPECT_LE(abs(out.f_c - 0.1307D), 0.0001D);
  EXPECT_LE(abs(out.r_f - 0.9271D), 0.0001D);
}

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/

class BasicPageMappingTestFixture: public ::testing::Test{
protected:
  SimpleSSD::ConfigReader* conf;
  FTL* p_ftl;
  PageMapping* p_pmap;
  SimpleSSD::DRAM::AbstractDRAM * p_dram;
  ConfigInfo* cfg_info;
  BasicPageMappingTestFixture(){
    conf = nullptr;
    p_ftl = nullptr;
    p_pmap = nullptr;
    p_dram = nullptr;
    cfg_info = nullptr;
  }
};

class PageMappingTestFixture: public BasicPageMappingTestFixture{
protected:
    static void SetUpTestCase()
    {
      //called before every testsuit
    }

    static void TearDownTestCase()
    {
        //called after every testsuit
    }

    void SetUp(){
      // called before every test
      remakeFTL(conf, p_ftl, p_pmap, p_dram, cfg_info);
      p_pmap->resetStatValues();
    }

    void TearDown(){
      // called after every test;
      clear_ptr(conf);
      clear_ptr(p_ftl);
      clear_ptr(p_dram);
      clear_ptr(cfg_info);
    }
};

TEST_F(PageMappingTestFixture, NewWriteTest){
  //NewWriteTest
  Request req = Request(ioUnitInPage);
  int write_pages = pageCount;
  req.ioFlag.set();
  uint64_t tick = 0LL;
  for(uint64_t i = 0; i<write_pages; ++i){
    req.lpn = i + cfg_info->nPagesToWarmup;
    p_pmap->write(req, tick);
  }
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(block_stat.compressUnitCount, 0);
  EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * (cfg_info->nPagesToWarmup + write_pages));
  EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * (cfg_info->nPagesToWarmup + write_pages));
  EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * (cfg_info->nPagesToWarmup  + write_pages));
  EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * (cfg_info->nPagesToWarmup + write_pages));
  EXPECT_EQ(p_pmap->stat.decompressCount, 0);
  EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
  EXPECT_EQ(p_pmap->stat.gcCount, 0);
  EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, 0);
  EXPECT_EQ(p_pmap->stat.reclaimedBlocks, 0);
  EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
  EXPECT_EQ(p_pmap->stat.totalWriteIoUnitCount, write_pages * ioUnitInPage);
}

TEST_F(PageMappingTestFixture, TrimTest){
  Request req = Request(ioUnitInPage);
  int trim_pages = pageCount;
  req.ioFlag.set();
  uint64_t tick = 0LL;
  for(uint64_t i = 0; i<trim_pages; ++i){
    req.lpn = i;
    p_pmap->trim(req, tick);
  }
  uint64_t valid_page = cfg_info->nPagesToWarmup - trim_pages;
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(block_stat.compressUnitCount, 0);
  EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * valid_page);
  EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * valid_page);
  EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * valid_page);
  EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * valid_page);
  EXPECT_EQ(p_pmap->stat.decompressCount, 0);
  EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
  EXPECT_EQ(p_pmap->stat.gcCount, 0);
  EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, 0);
  EXPECT_EQ(p_pmap->stat.reclaimedBlocks, 0);
  EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
  EXPECT_EQ(p_pmap->stat.totalWriteIoUnitCount, 0);
}

TEST_F(PageMappingTestFixture, OverWriteTest){
  Request req = Request(ioUnitInPage);
  int write_pages = pageCount * 100;
  req.ioFlag.set();
  uint64_t tick = 0LL;
  for(uint64_t i = 1; i<=write_pages; ++i){
    req.lpn = i;
    p_pmap->write(req, tick);
    if(i % pageCount == 0){
      std::cout <<"OverWriteTest-Finished pages: " << i << "/" << write_pages << "\n";
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
  utStatFile<<"unit_test_none ";
  outputUTStats(block_stat, p_pmap);
  printFTLInfo(p_pmap);
}

TEST_F(PageMappingTestFixture, GCTest){
  auto GCTest = [&](std::string test_name, SimpleSSD::CompressType compress_type){
    Request req = Request(ioUnitInPage);
    req.cd_info.offset = 0;
    req.cd_info.pDisk = createTestDisk(compress_type, cfg_info->nPagesToWarmup);
    EXPECT_GE(((SimpleSSD::CompressedDisk*)(req.cd_info.pDisk))->compress_unit_totalcnt, cfg_info->nPagesToWarmup * ioUnitInPage);
    int write_pages = pageCount * 100;
    req.ioFlag.set();
    uint64_t tick = 0LL;
    for(uint64_t i = 1; i<=write_pages; ++i){
      req.lpn = i;
      p_pmap->write(req, tick);
      if(i % pageCount == 0){
        std::cout <<"gcTest-Finished pages: " << i << "/" << write_pages << "\n";
      }
    }
    BlockStat block_stat = p_pmap->calculateBlockStat();
    EXPECT_EQ(p_pmap->stat.decompressCount, 0);
    EXPECT_GE(p_pmap->stat.failedCompressCout, 0);
    EXPECT_GT(p_pmap->stat.gcCount, 10);
    EXPECT_GE(p_pmap->stat.overwriteCompressUnitCount, 0);
    EXPECT_GT(p_pmap->stat.reclaimedBlocks, 90);
    EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
    EXPECT_GE(p_pmap->stat.totalWriteIoUnitCount, ioUnitInPage * write_pages);
    EXPECT_GT(block_stat.compressUnitCount, 0);
    EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
    EXPECT_LE(block_stat.validDataLength, block_stat.totalDataLength);
    EXPECT_LE(block_stat.validIoUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
    EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
    // print infos;
    utStatFile<<test_name<<" ";
    outputUTStats(block_stat, p_pmap);
    printFTLInfo(p_pmap);
    delete req.cd_info.pDisk;
  };
  GCTest("unit_test_lz4", SimpleSSD::CompressType::LZ4);
  GCTest("unit_test_lzma", SimpleSSD::CompressType::LZMA);
}

class PageMappingConsistencyTestFixture : public BasicPageMappingTestFixture{
protected:
  void SetUp() override{
    remakeFTL(conf, p_ftl, p_pmap, p_dram, cfg_info);
    p_pmap->resetStatValues();
  }
  void TearDown() override{
    clear_ptr(conf);
    clear_ptr(p_ftl);
    clear_ptr(p_dram);
    clear_ptr(cfg_info);
  }
};

TEST_F(PageMappingConsistencyTestFixture, PageMappingConsistencyTest){
  //TODO, Use test.cpp
}


