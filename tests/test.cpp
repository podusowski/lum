#include <mutex>
#include <stdexcept>
#include <thread>

#define REQUIRE(expr)                                                          \
  if (!(expr))                                                                 \
    throw std::runtime_error(#expr);

void increment(int &value, std::mutex &m) {
  std::unique_lock<std::mutex> guard{m};
  value++;
}

void test() {
  int value{0};
  std::mutex m;
  std::thread t{increment, std::ref(value), std::ref(m)};

  {
    std::unique_lock<std::mutex> guard{m};
    REQUIRE(0 == value);
  }

  t.join();
}

int main() { test(); }