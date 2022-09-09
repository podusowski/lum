#include "lum/log.hpp"
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
    lum::trace{} << "writing new value";
    value = 42;
  }};

  {
    std::unique_lock<lum::mutex> lock{mutex};
    lum::trace{} << "value: " << value;
    values.insert(value);
  }

  t.join();
}

int main() {
  // Boilerplate which ought to end up somewhere in the lib itself.
  lum::mutator mutator;
  std::set<int> values;

  for (auto i = 0; i < 5; i++) {
    lum::trace{} << "iteration has started";
    test(mutator, values);
    lum::trace{};
    mutator.next();
  }

  assert((values == std::set<int>{0, 42}));
}
