#pragma once

#include <iostream>
#include <mutex>

namespace lum {

struct trace {
  template <class T> trace &operator<<(T &&t) {
    std::cerr << t;
    return *this;
  }

  ~trace() { std::cerr << std::endl; }

private:
  inline static std::mutex _mutex;
  std::unique_lock<std::mutex> _lock{_mutex};
};

} // namespace lum
