#include "lum/lum.hpp"
#include <cassert>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>

void increment(int &value, lum::mutex &m) {
  std::unique_lock<lum::mutex> lock{m};
  std::cerr << "writing new value" << std::endl;
  value = 42;
}

void test(lum::mutator &mutator, std::set<int> &values) {
  int value{0};
  lum::mutex m{mutator};
  std::thread t{increment, std::ref(value), std::ref(m)};

  {
    std::unique_lock<lum::mutex> lock{m};
    std::cerr << "value: " << value << std::endl; // Will it be 0 or 1?
    values.insert(value);
  }

  t.join();
}

int main() {
  // Boilerplate which ought to end up somewhere in the lib itself.
  lum::mutator mutator;
  std::set<int> values;

  std::cerr << "characterization pass" << std::endl;
  test(mutator, values);
  mutator.mutation_pass();
  std::cerr << std::endl;

  for (auto i = 0; i < 10; i++) {
    mutator.next();
    std::cerr << "iteration has started" << std::endl;
    test(mutator, values);
    std::cerr << std::endl;
  }

  assert((values == std::set<int>{0, 42}));
}
