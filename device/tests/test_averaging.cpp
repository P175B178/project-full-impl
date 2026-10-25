#include "AveragingBuffer.h"

#include <gtest/gtest.h>
#include <stdexcept>

using namespace warden;

TEST(AveragingBufferTest, ThrowsOnZeroCapacity) { EXPECT_THROW(AveragingBuffer<float>{0}, std::invalid_argument); }

TEST(AveragingBufferTest, EmptyBufferReturnsNullopt) {
  AveragingBuffer<float> buf{5};
  EXPECT_FALSE(buf.average().has_value());
}

TEST(AveragingBufferTest, SingleValueAverageReturnsThatValue) {
  AveragingBuffer<float> buf{5};
  buf.push(23.4F);
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_FLOAT_EQ(buf.average().value(), 23.4F);
}

TEST(AveragingBufferTest, AverageOfMultipleValuesIsCorrect) {
  AveragingBuffer<float> buf{4};
  buf.push(10.0F);
  buf.push(20.0F);
  buf.push(30.0F);
  buf.push(40.0F);
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_FLOAT_EQ(buf.average().value(), 25.0F);
}

TEST(AveragingBufferTest, OldValuesAreEvictedOnceFull) {
  AveragingBuffer<float> buf{3};
  buf.push(10.0F);
  buf.push(10.0F);
  buf.push(10.0F);
  buf.push(40.0F); // evicts first 10.0 — window: [10, 10, 40] -> average = 20
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_FLOAT_EQ(buf.average().value(), 20.0F);
}

TEST(AveragingBufferTest, CapacityOfOneAlwaysReturnsLastPushedValue) {
  AveragingBuffer<float> buf{1};
  buf.push(10.0F);
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_FLOAT_EQ(buf.average().value(), 10.0F);
  buf.push(99.0F);
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_FLOAT_EQ(buf.average().value(), 99.0F);
}

TEST(AveragingBufferTest, WorksWithIntegerTypes) {
  AveragingBuffer<int> buf{4};
  buf.push(10);
  buf.push(20);
  buf.push(30);
  buf.push(40);
  ASSERT_TRUE(buf.average().has_value());
  EXPECT_EQ(buf.average().value(), 25);
}
