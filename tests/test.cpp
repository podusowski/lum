#include <mutex>
#include <stdexcept>
#include <thread>
#include "lum/lum.hpp"

#define REQUIRE(expr)                                                          \
  if (!(expr))                                                                 \
    throw std::runtime_error(#expr);

void increment(int &value, lum::mutex &m) {
  std::unique_lock<lum::mutex> guard{m};
  value++;
}

void test() {
  int value{0};
  lum::mutex m;
  std::thread t{increment, std::ref(value), std::ref(m)};

  {
    std::unique_lock<lum::mutex> guard{m};
    REQUIRE(0 == value);
  }

  t.join();
}

int main() { test(); }