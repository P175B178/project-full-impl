#pragma once

#include <cstddef>
#include <deque>
#include <numeric>
#include <optional>
#include <stdexcept>

namespace warden {

/// Keeps the last `capacity` values and computes their average.
template <typename T> class AveragingBuffer {
public:
  explicit AveragingBuffer(std::size_t capacity)
        : m_capacity{capacity} {
    if (capacity == 0) {
      throw std::invalid_argument("Averaging window must be > 0");
    }
  }

  /// Add a new value, evicting the oldest if the buffer is full.
  void push(T value) {
    m_values.push_back(value);
    if (m_values.size() > m_capacity) {
      m_values.pop_front();
    }
  }

  /// The mean of the stored values; std::nullopt if there are none.
  [[nodiscard]] std::optional<T> average() const {
    if (m_values.empty()) {
      return std::nullopt;
    }
    return std::accumulate(m_values.begin(), m_values.end(), T{}) / static_cast<T>(m_values.size());
  }

private:
  std::size_t m_capacity;
  std::deque<T> m_values;
};

} // namespace warden
