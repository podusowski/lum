#include <iostream>
#include <mutex>
#include <thread>

int main() {
  int value{0};
  std::mutex mutex;

  // Supplementary thread.
  std::thread t{[&] {
    std::unique_lock<std::mutex> lock{mutex};
    value = 42;
  }};

  // Main thread.
  {
    std::unique_lock<std::mutex> lock{mutex};
    std::cerr << "value: " << value << std::endl; // 0 or 42?
  }

  t.join();
}