#include <catch2/catch_test_macros.hpp>
#include <mutex>
#include <thread>

void increment(int &value, std::mutex &m) {
  std::unique_lock<std::mutex> guard{m};
  value++;
}

TEST_CASE("test") {
  int value{0};
  std::mutex m;
  std::thread t{increment, std::ref(value), std::ref(m)};
  REQUIRE(1 == value);
  t.join();
}
