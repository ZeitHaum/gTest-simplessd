#pragma once

#include <thread>
#include <memory>
#include <errno.h>
#include "def.hh"
#include "utils.hh"
// #include "utiterator.hh"
#include "random_generator.hh"
// #include "consistency.hh"

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

TEST(RandGenTest, SrandSeedTest){
  srand(RANDOM_SEED);
  int x = rand();
  srand(RANDOM_SEED);
  int y = rand();
  EXPECT_EQ(x, y);
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
  BasicPageMappingTestFixture();
  void SetUp() override;
  void TearDown() override;
};

class PageMappingTestFixture: public BasicPageMappingTestFixture{
protected: 
  void SetUp() override;
  void TearDown() override;
  void init();
  void clear();
  void reset();
  void GCOrdinaryTest(uint32_t write_pages);
  // void GCCompressTest(UTTestInfo& test_info);
};

// TEST_F(BasicPageMappingTestFixture, NewWriteTest){
//   //NewWriteTest
//   Request req = Request(ioUnitInPage);
//   int write_pages = pageCount;
//   req.ioFlag.set();
//   uint64_t tick = 0LL;
//   for(uint64_t i = 0; i<write_pages; ++i){
//     req.lpn = i + cfg_info->nPagesToWarmup;
//     p_pmap->write(req, tick);
//   }
//   // BlockStat block_stat = p_pmap->calculateBlockStat();
//   // EXPECT_EQ(block_stat.compressUnitCount, 0);
//   // EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * (cfg_info->nPagesToWarmup + write_pages));
//   // EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * (cfg_info->nPagesToWarmup + write_pages));
//   // EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * (cfg_info->nPagesToWarmup  + write_pages));
//   // EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * (cfg_info->nPagesToWarmup + write_pages));
//   EXPECT_EQ(p_pmap->stat.decompressCount, 0);
//   EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
//   EXPECT_EQ(p_pmap->stat.gcCount, 0);
//   EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, 0);
//   EXPECT_EQ(p_pmap->stat.reclaimedBlocks, 0);
//   EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
//   EXPECT_EQ(p_pmap->stat.totalWriteIoUnitCount, write_pages * ioUnitInPage);
// }

// TEST_F(BasicPageMappingTestFixture, TrimTest){
//   Request req = Request(ioUnitInPage);
//   int trim_pages = pageCount;
//   req.ioFlag.set();
//   uint64_t tick = 0LL;
//   for(uint64_t i = 0; i<trim_pages; ++i){
//     req.lpn = i;
//     p_pmap->trim(req, tick);
//   }
//   uint64_t valid_page = cfg_info->nPagesToWarmup - trim_pages;
//   // BlockStat block_stat = p_pmap->calculateBlockStat();
//   // EXPECT_EQ(block_stat.compressUnitCount, 0);
//   // EXPECT_EQ(block_stat.totalDataLength, ioUnitInPage * ioUnitSize * valid_page);
//   // EXPECT_EQ(block_stat.validDataLength, ioUnitInPage * ioUnitSize * valid_page);
//   // EXPECT_EQ(block_stat.validIoUnitCount, ioUnitInPage * valid_page);
//   // EXPECT_EQ(block_stat.totalUnitCount, ioUnitInPage * valid_page);
//   EXPECT_EQ(p_pmap->stat.decompressCount, 0);
//   EXPECT_EQ(p_pmap->stat.failedCompressCout, 0);
//   EXPECT_EQ(p_pmap->stat.gcCount, 0);
//   EXPECT_EQ(p_pmap->stat.overwriteCompressUnitCount, 0);
//   EXPECT_EQ(p_pmap->stat.reclaimedBlocks, 0);
//   EXPECT_EQ(p_pmap->stat.totalReadIoUnitCount, 0);
//   EXPECT_EQ(p_pmap->stat.totalWriteIoUnitCount, 0);
// }

TEST_F(PageMappingTestFixture, OverWriteTest){
  // UTTestIterator ut_iter;
  // ut_iter.init(pageCount * 100);
  init();
  SimpleSSD::initCPU(*conf);
  GCOrdinaryTest(pageCount * 100);
  // for(uint16_t i = 0; i<wrpage_split_factor; ++i){
  // }
}

// TEST_F(PageMappingTestFixture, GCTest){
//   UTTestIterator ut_iter = UTTestIterator(false);
//   ut_iter.init(pageCount * 100);
//   while(!ut_iter.is_end()){
//     UTTestInfo ut_info = ut_iter.getnextTestInfo();
//     GCCompressTest(ut_info);
//     std::cout <<"Finished test:"<< ut_info.getTestName() << std::endl;
//   }
// }

// TEST(RegularTest, CompressRatioTest){
//   SimpleSSD::LZ4Compressor lz4_comp = SimpleSSD::LZ4Compressor();
//   SimpleSSD::LzmaCompressor lzma_comp = SimpleSSD::LzmaCompressor();
//   uint8_t lz4input[ioUnitSize];
//   uint8_t lzmainput[ioUnitSize];
//   uint64_t lz4_totallen = 0;
//   uint64_t lzma_totallen = 0;
//   const int ITER_NUM = 10000;
//   srand(RANDOM_SEED);
//   for(uint32_t itn = 1; itn <= ITER_NUM; ++itn){
//     for(uint32_t i = 0; i<ioUnitSize; ++i){
//       lz4input[i] = getRandomByte();
//     }
//     memcpy(lzmainput, lz4input, ioUnitSize);
//     uint64_t lz4_destlen = 0;
//     uint64_t lzma_destlen = 0;
//     lz4_comp.compress(lz4input, ioUnitSize, lz4_destlen);
//     lzma_comp.compress(lzmainput, ioUnitSize, lzma_destlen);
//     lz4_destlen = min(ioUnitSize, lz4_destlen);
//     lzma_destlen = min(ioUnitSize, lzma_destlen);
//     lz4_totallen += lz4_destlen;
//     lzma_totallen += lzma_destlen;
//     if(itn % (ITER_NUM / 10) == 0){
//       EXPECT_EQ(get_percent(lz4_totallen, itn*ioUnitSize), 100);
//       EXPECT_EQ(get_percent(lzma_totallen, itn*ioUnitSize), 100);
//     }
//   }
// }

// TEST_F(PageMappingTestFixture, GCCompressRatioTest){
//   //Get Compress Data
//   uint8_t test_data[65536];
//   memset(test_data, 0, 65536);
//   srand(RANDOM_SEED);
//   for(uint32_t i = 0; i<64; ++i){
//       uint32_t pos = rand() % 4096;
//       test_data[pos] = rand() % 256;
//   }
//   for(uint32_t i = 1; i<16; ++i){
//     memcpy(test_data + i*4096, test_data, 4096);
//   }
//   //Get Compress Ratio
//   double lz4_percent = 0.0D;
//   double lzma_percent = 0.0D;
//   uint64_t lz4_totallen = 0;
//   uint64_t lzma_totallen = 0;
//   uint64_t lz4_destlen = 0;
//   uint64_t lzma_destlen = 0;
//   SimpleSSD::LZ4Compressor lz4_comp = SimpleSSD::LZ4Compressor();
//   SimpleSSD::LzmaCompressor lzma_comp = SimpleSSD::LzmaCompressor();
//   lz4_comp.compress(test_data, 4096, lz4_destlen);
//   lzma_comp.compress(test_data, 4096, lzma_destlen);
//   lz4_percent = get_percent(lz4_destlen, 4096);
//   lzma_percent = get_percent(lzma_destlen, 4096);
//   //Check Average
//   for(uint32_t i = 0; i<16; ++i){
//     lz4_comp.compress(test_data + i * 4096, 4096, lz4_destlen);
//     lzma_comp.compress(test_data + i * 4096, 4096, lzma_destlen);
//     lz4_totallen += lz4_destlen;
//     lzma_totallen += lzma_destlen;
//   }
//   EXPECT_EQ(get_percent(lz4_totallen, 4096*16), lz4_percent);
//   EXPECT_EQ(get_percent(lzma_totallen, 4096*16), lzma_percent);
//   std::cout << "Standard lz4_percent:" << lz4_percent << "\n";
//   std::cout << "Standard lzma_percent:" << lzma_percent << "\n";
//   //SimpleSSD Compress
//   UTTestInfo ut_info = UTTestInfo(SimpleSSD::CompressType::LZ4, DiskWritePolicy::CUSTOM, DiskInitPolicy::ALL_ZERO, pageCount * 100, test_data);
//   GCCompressTest(ut_info);
//   std::cout <<"Finished test:"<< ut_info.getTestName() << std::endl;
//   ut_info.comptype = SimpleSSD::CompressType::LZMA;
//   GCCompressTest(ut_info);
//   std::cout <<"Finished test:"<< ut_info.getTestName() << std::endl;
// }