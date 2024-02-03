//author: ZeitHaum
#include "ftl/page_mapping.hh"
#include "dram/simple.hh"
#include "gtest/gtest.h"
#include "random_generator.cc"

using namespace SimpleSSD::FTL;

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

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/

struct ConfigInfo{
  uint64_t nTotalLogicalPages;
  uint64_t nPagesToWarmup;
  uint64_t nPagesToInvalidate;
};

class PageMappingTestFixture: public ::testing::Test{
protected:
    static void SetUpTestCase()
    {
        //called before every testsuit
        conf = new SimpleSSD::ConfigReader();
        const string* s = &simple_config_file;
        if(test_cfg==TestConfig::SIMPLE){
            s = &simple_config_file;
            pageCount = 68;
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

    static void TearDownTestCase()
    {
        //called after every testsuit
        delete conf;
        delete p_ftl;
        delete p_dram;
    }

    void SetUp(){
      // called before every test
    }

    void TearDown(){
      // called after every test;
      p_pmap->resetStatValues();
    }

    static SimpleSSD::ConfigReader* conf;
    static FTL* p_ftl;
    static PageMapping* p_pmap;
    static SimpleSSD::DRAM::AbstractDRAM * p_dram;
    static ConfigInfo* cfg_info;
};

SimpleSSD::ConfigReader* PageMappingTestFixture::conf = nullptr;
FTL* PageMappingTestFixture::p_ftl = nullptr;
PageMapping* PageMappingTestFixture::p_pmap = nullptr;
SimpleSSD::DRAM::AbstractDRAM * PageMappingTestFixture::p_dram = nullptr;
ConfigInfo* PageMappingTestFixture::cfg_info = nullptr;

TEST_F(PageMappingTestFixture, InitTest){
    //Test Initialize
    //check total stat
    BlockStat block_stat = p_pmap->calculateBlockStat();
    EXPECT_EQ(block_stat.compressUnitCount, 0);
    EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
    EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
    EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
    EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
    EXPECT_EQ(p_pmap->stat.decompressCount, 0);
    EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
    EXPECT_EQ(p_pmap->stat.gcCount, 0);
    EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, cfg_info->nPagesToInvalidate);
    EXPECT_EQ(p_pmap->stat.reclaimedBlocks, 0);
    EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
    EXPECT_EQ(p_pmap->stat.totalWriteIoUnitCount, (cfg_info->nPagesToInvalidate + cfg_info->nPagesToWarmup)*ioUnitInPage);
}

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
  int write_pages = pageCount;
  req.ioFlag.set();
  uint64_t tick = 0LL;
  for(uint64_t i = 0; i<write_pages; ++i){
    req.lpn = i + cfg_info->nPagesToWarmup;
    p_pmap->trim(req, tick);
  }
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(block_stat.compressUnitCount, 0);
  EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
  EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * cfg_info->nPagesToWarmup);
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
}

TEST_F(PageMappingTestFixture, GCTest){
  Request req = Request(ioUnitInPage);
  req.cd_info.offset = 0;
  req.cd_info.pDisk = new SimpleSSD::CompressedDisk();
  req.cd_info.pDisk->open(img_file, cfg_info->nPagesToWarmup * ioUnitInPage* ioUnitSize, ioUnitSize);
  ((SimpleSSD::CompressedDisk*)(req.cd_info.pDisk))->init(ioUnitSize, SimpleSSD::CompressType::LZMA);
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
}