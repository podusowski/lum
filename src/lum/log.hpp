#pragma once

#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

namespace lum {

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

} // namespace lum
