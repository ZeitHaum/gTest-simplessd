//author: ZeitHaum
#include "ftl/page_mapping.hh"
#include "gtest/gtest.h"
#include "random_generator.cc"

using namespace SimpleSSD::FTL;

const std::string config_file = "./simplessd/config/samsung_983dct_1.92tb.cfg";

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/
//begin Test blockStat, Related commit [dev 8d6c014] Finish blockStat
class PageMappingTestFixture: public ::testing::Test{
protected:
    void SetUp() override
    {
        //called before every testsuit
        conf = new ConfigReader();
        if(!conf->init(config_file)){
            assert(0 && "Failed to laod file.");
        }
        p_dram = new SimpleSSD::DRAM::SimpleDRAM(conf);
        p_ftl = new FTL(conf, p_dram);
    }

    void TearDown() override
    {
        //called after every testsuit
        delete conf;
        delete pFtl;
    }

    SimpleSSD::ConfigReader* conf;
    FTL* p_ftl;
    SimpleSSD::DRAM::AbstractDRAM * p_dram;
};

TEST_F(PageMappingTestFixture, InitTest){
    //Samply Test SetUp Function(Initialize Test);
    ASSERT_TRUE(true);
}


