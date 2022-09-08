#pragma once

#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

namespace lum {

struct trace {
  trace() { std::cerr << "thread:" << std::this_thread::get_id() << ": "; }
  ~trace() { std::cerr << std::endl; }

  template <class T> trace &operator<<(T &&t) {
    std::cerr << t;
    return *this;
  }

private:
  inline static std::mutex _mutex;
  std::unique_lock<std::mutex> _lock{_mutex};
};

} // namespace lum
