#include <iostream>
#include <thread>

int main() {
  int value{0};

  // Supplementary thread.
  std::thread t{[&] { value = 42; }};

  // Main thread.
  std::cerr << "value: " << value << std::endl; // 0 or 42?

  t.join();
}
