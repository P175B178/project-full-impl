#include "BlockingQueue.h"

#include <gtest/gtest.h>

#include <chrono>
#include <future>
#include <stop_token>
#include <thread>

using namespace warden;
using namespace std::chrono_literals;

// A default-constructed stop_token can never be stopped.
static const std::stop_token noStop;

TEST(BlockingQueueTest, PopReturnsItemsInFifoOrder) {
  BlockingQueue<int> queue;
  queue.push(1);
  queue.push(2);
  queue.push(3);
  EXPECT_EQ(queue.pop(noStop), 1);
  EXPECT_EQ(queue.pop(noStop), 2);
  EXPECT_EQ(queue.pop(noStop), 3);
}

TEST(BlockingQueueTest, PopBlocksUntilItemIsPushed) {
  BlockingQueue<int> queue;
  auto popped = std::async(std::launch::async, [&queue] { return queue.pop(noStop); });

  EXPECT_EQ(popped.wait_for(50ms), std::future_status::timeout);
  queue.push(42);
  ASSERT_EQ(popped.wait_for(5s), std::future_status::ready);
  EXPECT_EQ(popped.get(), 42);
}

TEST(BlockingQueueTest, StopUnblocksWaitingPop) {
  BlockingQueue<int> queue;
  std::stop_source source;
  auto popped = std::async(std::launch::async, [&queue, &source] { return queue.pop(source.get_token()); });

  EXPECT_EQ(popped.wait_for(50ms), std::future_status::timeout);
  source.request_stop();
  ASSERT_EQ(popped.wait_for(5s), std::future_status::ready);
  EXPECT_FALSE(popped.get().has_value());
}

TEST(BlockingQueueTest, PopReturnsQueuedItemsBeforeHonouringStop) {
  BlockingQueue<int> queue;
  std::stop_source source;
  queue.push(1);
  source.request_stop();
  EXPECT_EQ(queue.pop(source.get_token()), 1);
  EXPECT_FALSE(queue.pop(source.get_token()).has_value());
}

TEST(BlockingQueueTest, DeliversEveryItemAcrossThreads) {
  constexpr int itemCount = 1000;
  BlockingQueue<int> queue;

  std::jthread producer{[&queue] {
    for (int item = 0; item < itemCount; ++item) {
      queue.push(item);
    }
  }};

  for (int expected = 0; expected < itemCount; ++expected) {
    EXPECT_EQ(queue.pop(noStop), expected);
  }
}
