//author: ZeitHaum

#define private public
#include "ftl/common/block.hh"
#undef private
#include "gtest/gtest.h"

using namespace SimpleSSD::FTL;

/**
 * The following tests are all base on my project Simplessd:
 * https://github.com/ZeitHaum/Simplessd.git (branch dev)
 * Most code is in ftl/.
*/
//begin Test blockStat, Related commit [dev 8d6c014] Finish blockStat
TEST(FTLBlockTest, setStaticAttrTest){
    EXPECT_EQ(Block::iounitSize, 0);
    EXPECT_EQ(Block::iounitSize, 0);
    Block::setStaticAttr(16, 8);
    EXPECT_EQ(Block::iounitSize, 16);
    EXPECT_EQ(Block::maxCompressedPageCount, 8);
}

// Block test_block = Block(0, 768, 16);
// TEST(FTLBlockTest, updateStatWriteTest){
//     test_block.blockstat.reset();
//     EXPECT_EQ(Block::iounitSize, 16);
//     EXPECT_EQ(Block::maxCompressedPageCount, 8);
// }