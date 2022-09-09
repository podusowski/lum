#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <ostream>
#include <thread>

namespace lum::log {

template <class T> struct range_t { const T &value; };

template <class T> std::ostream &operator<<(std::ostream &os, range_t<T> c) {
  for (auto it = std::cbegin(c.value); it < std::cend(c.value); it++) {
    if (it == std::cbegin(c.value))
      os << *it;
    else
      os << ", " << *it;
  }
  return os;
}

// Wraps a container so it can be streamed.
template <class T> range_t<T> range(const T &c) {
  // Do we want to use C++17's CTAD instead?
  return range_t<T>{c};
}

struct trace {
  trace() : enabled(std::getenv("LUM_TRACE") != nullptr) {
    if (enabled)
      std::cerr << "thread:" << std::this_thread::get_id() << ": ";
  }

  ~trace() {
    if (enabled)
      std::cerr << std::endl;
  }

  template <class T> trace &operator<<(T &&t) {
    if (enabled)
      std::cerr << t;
    return *this;
  }

private:
  bool enabled{true};
  inline static std::mutex _mutex;
  std::unique_lock<std::mutex> _lock{_mutex};
};

} // namespace lum::log
