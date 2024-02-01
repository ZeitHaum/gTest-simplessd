//author: ZeitHaum

#define private public
#include "ftl/common/block.hh"
#undef private
#include "gtest/gtest.h"

using namespace SimpleSSD;

TEST(TestMake, Test){
    Bitset b = Bitset(24);
    EXPECT_EQ(b.allocSize, 3);
}

