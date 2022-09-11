#include "lum/assert.hpp"
#include "lum/log.hpp"
#include "lum/lum.hpp"

#include <cassert>
#include <future>
#include <mutex>
#include <set>
#include <stdexcept>
#include <thread>

struct my_supposedly_threadsafe_storage {
  int get_a() const {
    std::unique_lock<lum::mutex> lock{m_for_a};
    return a;
  }

  void set_a(int value) {
    std::unique_lock<lum::mutex> lock{m_for_a};
    a = value;
  }

  int get_b() const {
    std::unique_lock<lum::mutex> lock{m_for_b};
    return b;
  }

  void set_b(int value) {
    std::unique_lock<lum::mutex> lock{m_for_b};
    b = value;
  }

private:
  mutable lum::mutex m_for_a;
  int a{};

  mutable lum::mutex m_for_b;
  int b{};
};

void test(std::set<std::tuple<int, int>> &values) {
  my_supposedly_threadsafe_storage storage;

  std::thread writer{[&] {
    storage.set_a(3);
    storage.set_b(3);
  }};

  auto sum = std::async(std::launch::async,
                        [&] { return storage.get_a() + storage.get_b(); });

  auto product = std::async(std::launch::async,
                            [&] { return storage.get_a() * storage.get_b(); });

  const auto result = std::make_tuple(sum.get(), product.get());
  values.insert(result);

  std::cerr << "sum: " << std::get<0>(result)
            << ", product: " << std::get<1>(result) << std::endl;

  writer.join();
}

namespace std {

std::ostream &operator<<(std::ostream &os, std::tuple<int, int> value) {
  return os << '(' << std::get<0>(value) << ", " << std::get<1>(value) << ')';
}

} // namespace std

int main() {
  // Boilerplate which ought to end up somewhere in the lib itself.
  lum::mutator &mutator = lum::global_mutator();
  std::set<std::tuple<int, int>> values;

  do {
    test(values);
  } while (mutator.next());

  lum::assert_eq(values, std::set<std::tuple<int, int>>{
                             {0, 0}, {3, 0}, {0, 9}, {3, 9}, {6, 0}, {6, 9}});
}
