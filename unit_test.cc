//author: ZeitHaum

#define private public
#include "ftl/common/block.hh"
#undef private
#include "gtest/gtest.h"
#include "random_generator.cc"

using namespace SimpleSSD::FTL;
using namespace SimpleSSD;

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/
//begin Test blockStat, Related commit [dev 8d6c014] Finish blockStat
// TEST(FTLBlockTest, setStaticAttrTest){
//     EXPECT_EQ(Block::iounitSize, 0);
//     EXPECT_EQ(Block::iounitSize, 0);
//     Block::setStaticAttr(4096, 8);
//     EXPECT_EQ(Block::iounitSize, 4096);
//     EXPECT_EQ(Block::maxCompressedPageCount, 8);
// }

// TEST(FTLBlockTest, updateStatWriteTest){
//     Block::setStaticAttr(4096, 8);
//     Block uncomp_block = Block(0, 768, 16);
//     Block comp_block = Block(0, 768, 16);
//     uncomp_block.blockstat.reset();
//     comp_block.blockstat.reset();
//     //random generate samples
//     Bitset validMask = Bitset(8);
//     std::vector<uint32_t> lens(8, 0);
//     /*
//     * Uncompress Test
//     */
//     for(int i = 0; i<8; ++i){
//         lens[i] = 4096;
//     }
//     BlockStat std_{768*16*4096, 768*16*4096, 768*16, 0,768*16};
//     validMask.set(0);
//     for(uint16_t i = 0; i<768*16; ++i){
//         uncomp_block.updateStatWrite(lens, validMask);
//     }
//     EXPECT_EQ(std_, uncomp_block.blockstat);

//     /*
//     * compress Test
//     */
//     std_.reset();
//     for(uint16_t i = 0; i<768 * 16; ++i){
//         //both compress and uncompress
//         getRandomBitset(validMask);
//         getRamdomVector<uint32_t>(lens, 1, 4096);
//         for(uint16_t j = 0; j<8; ++j){
//             if(validMask.test(j)) {
//                 ++std_.totalUnitCount;
//                 std_.totalDataLength += 4096;
//                 std_.validDataLength += lens[j];
//                 if(lens[j] < 4096) ++std_.compressUnitCount;
//             }
//         }
//         if(validMask.any()) ++std_.validIoUnitCount;
//         comp_block.updateStatWrite(lens, validMask);
//         EXPECT_EQ(std_, comp_block.blockstat);
//     }
// }

