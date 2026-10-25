#include "Range.h"

#include <gtest/gtest.h>

using namespace warden;

static constexpr Range range{.min = 0.0F, .max = 50.0F};

TEST(RangeTest, ContainsValuesInside) { EXPECT_TRUE(range.contains(25.0F)); }

TEST(RangeTest, BoundariesAreIncluded) {
  EXPECT_TRUE(range.contains(0.0F));
  EXPECT_TRUE(range.contains(50.0F));
}

TEST(RangeTest, RejectsValuesOutside) {
  EXPECT_FALSE(range.contains(-0.1F));
  EXPECT_FALSE(range.contains(50.1F));
}
