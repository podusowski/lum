#include "lum/lum.hpp"
#include <cassert>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>

void test(lum::mutator &mutator, std::set<int> &values) {
  int value{0};
  lum::mutex mutex{mutator};

  std::thread t{[&] {
    std::unique_lock<lum::mutex> lock{mutex};
    std::cerr << "writing new value" << std::endl;
    value = 42;
  }};

  {
    std::unique_lock<lum::mutex> lock{mutex};
    std::cerr << "value: " << value << std::endl;
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
  mutator.next();
  std::cerr << std::endl;

  for (auto i = 0; i < 10; i++) {
    mutator.next();
    std::cerr << "iteration has started" << std::endl;
    test(mutator, values);
    std::cerr << std::endl;
  }

  assert((values == std::set<int>{0, 42}));
}
