//author: ZeitHaum
#include "ftl/page_mapping.hh"
#include "dram/simple.hh"
#include "gtest/gtest.h"
#include "random_generator.cc"

using namespace SimpleSSD::FTL;

const std::string config_file = "./simplessd/config/samsung_983dct_1.92tb.cfg";

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/

/**
 * Check Functions
*/

bool checkstats(const BlockStat& blockstat){
  //check block stats
  uint64_t ioUnitInPage = 16;
  uint64_t pageCount = 768;
  if(blockstat.totalDataLength % Block::iounitSize !=0){
    return false;
  }
  else if(blockstat.validDataLength > blockstat.totalDataLength || blockstat.validDataLength > ioUnitInPage * pageCount * Block::iounitSize){
    return false;
  }
  else if(blockstat.totalUnitCount != blockstat.totalDataLength / Block::iounitSize){
    return false;
  }
  else if(blockstat.compressUnitCount > blockstat.totalUnitCount){
    return false;
  }
  else if(blockstat.validIoUnitCount > blockstat.totalUnitCount || blockstat.validIoUnitCount > ioUnitInPage * pageCount){
    return false;
  }
  else if((blockstat.totalDataLength - blockstat.validDataLength) * (blockstat.compressUnitCount) < (blockstat.totalUnitCount - blockstat.validIoUnitCount) * (blockstat.compressUnitCount * Block::iounitSize)){
    // check the ratio of fragment is non-negative.
    return false;
  }
  return true;
}

//begin Test blockStat, Related commit [dev 8d6c014] Finish blockStat
class PageMappingTestFixture: public ::testing::Test{
protected:
    static void SetUpTestCase()
    {
        //called before every testsuit
        conf = new SimpleSSD::ConfigReader();
        if(!conf->init(config_file)){
            assert(0 && "Failed to laod file.");
        }
        p_dram = new SimpleSSD::DRAM::SimpleDRAM(*conf);
        p_ftl = new FTL(*conf, p_dram);
        p_pmap = (PageMapping*)p_ftl->pFTL;
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
    }

    static SimpleSSD::ConfigReader* conf;
    static FTL* p_ftl;
    static PageMapping* p_pmap;
    static SimpleSSD::DRAM::AbstractDRAM * p_dram;
};

SimpleSSD::ConfigReader* PageMappingTestFixture::conf = nullptr;
FTL* PageMappingTestFixture::p_ftl = nullptr;
PageMapping* PageMappingTestFixture::p_pmap = nullptr;
SimpleSSD::DRAM::AbstractDRAM * PageMappingTestFixture::p_dram = nullptr;

TEST_F(PageMappingTestFixture, InitTest){
    //Samply Test SetUp Function(Initialize Test);
    BlockStat block_stat = p_pmap->calculateBlockStat();
    EXPECT_EQ(block_stat.compressUnitCount, 0);
    EXPECT_EQ(block_stat.totalDataLength, 768LL * 16 * 4096 * 9537);
    EXPECT_EQ(block_stat.validDataLength, 768LL * 16 * 4096 * 9537);
    EXPECT_EQ(block_stat.validIoUnitCount, 768LL * 16 * 9537);
    EXPECT_EQ(block_stat.totalUnitCount, 768LL * 16 * 9537);
}

TEST_F(PageMappingTestFixture, WriteTest){
  //Test Write
  BlockStat block_stat = p_pmap->calculateBlockStat();
  EXPECT_EQ(block_stat.compressUnitCount, 0);
  EXPECT_EQ(block_stat.totalDataLength, 768LL * 16 * 4096 * 9537);
  EXPECT_EQ(block_stat.validDataLength, 768LL * 16 * 4096 * 9537);
  EXPECT_EQ(block_stat.validIoUnitCount, 768LL * 16 * 9537);
  EXPECT_EQ(block_stat.totalUnitCount, 768LL * 16 * 9537);
}


