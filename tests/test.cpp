#include "lum/lum.hpp"
#include <mutex>
#include <stdexcept>
#include <thread>

void increment(int &value, lum::mutex &m) {
  std::unique_lock<lum::mutex> lock{m};
  value++;
}

void test(lum::mutator &mutator) {
  int value{0};
  lum::mutex m{mutator};
  std::thread t{increment, std::ref(value), std::ref(m)};

  {
    std::unique_lock<lum::mutex> lock{m};
    std::cerr << "value: " << value << std::endl; // Will it be 0 or 1?
  }

  t.join();
}

int main() {
  // Boilerplate which ought to end up somewhere in the lib itself.
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
