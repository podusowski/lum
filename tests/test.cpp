#include "lum/log.hpp"
#include "lum/lum.hpp"
#include <cassert>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>

void test(std::set<int> &values) {
  int value{0};
  lum::mutex mutex;

  std::thread t{[&] {
    std::unique_lock<lum::mutex> lock{mutex};
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
  lum::mutator &mutator = lum::global_mutator();
  std::set<int> values;

  do {
    test(values);
  } while (mutator.next());

  assert((values == std::set<int>{0, 42}));
}
