#include "lum/lum.hpp"
#include <mutex>
#include <stdexcept>
#include <thread>

#define REQUIRE(expr)                                                          \
  if (!(expr))                                                                 \
    throw std::runtime_error(#expr);

void increment(int &value, lum::mutex &m) {
  std::unique_lock<lum::mutex> guard{m};
  value++;
}

void test(lum::mutator &mutator) {
  int value{0};
  lum::mutex m{mutator};
  std::thread t{increment, std::ref(value), std::ref(m)};

  {
    std::unique_lock<lum::mutex> guard{m};
    //REQUIRE(0 == value);
    std::cerr << "value: " << value << std::endl;
  }

  t.join();
}

int main() {
  lum::mutator mutator;

  std::cerr << "characterization pass" << std::endl;
  test(mutator);
  mutator.mutation_pass();
  std::cerr << std::endl;

  for (auto i = 0; i < 10; i++) {
    mutator.next();
    std::cerr << "iteration has started" << std::endl;
    test(mutator);
    std::cerr << std::endl;
  }
}
