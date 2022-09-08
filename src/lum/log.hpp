#pragma once

#include <iostream>
#include <mutex>

namespace lum {

struct synced_cerr {
  template <class T> synced_cerr &operator<<(T &&t) {
    std::cerr << t;
    return *this;
  }

  ~synced_cerr() { std::cerr << std::endl; }

private:
  inline static std::mutex _mutex;
  std::unique_lock<std::mutex> _lock{_mutex};
};

} // namespace lum
