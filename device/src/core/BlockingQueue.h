#pragma once

#include <condition_variable>
#include <mutex>
#include <optional>
#include <queue>
#include <stop_token>
#include <utility>

namespace warden {

/// Thread-safe blocking queue: push() wakes the consumer, pop() waits for an item or a stop
/// request.
template <typename T> class BlockingQueue {
public:
  void push(T item) {
    {
      std::scoped_lock lock{m_mutex};
      m_queue.push(std::move(item));
    }
    m_itemAvailable.notify_one();
  }

  /// Returns the next item, or std::nullopt once a stop is requested and the queue is empty.
  [[nodiscard]] std::optional<T> pop(const std::stop_token &stop) {
    std::unique_lock lock{m_mutex};
    // condition_variable_any can wait on a stop_token: this ends when the predicate is true or a
    // stop is requested.
    if (!m_itemAvailable.wait(lock, stop, [this] { return !m_queue.empty(); })) {
      return std::nullopt;
    }

    T item = std::move(m_queue.front());
    m_queue.pop();
    return item;
  }

private:
  std::queue<T> m_queue;
  std::mutex m_mutex;
  std::condition_variable_any m_itemAvailable; // notified by push()
};

} // namespace warden
