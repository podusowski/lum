#include <iostream>
#include <thread>

int main() {
  int value{0};

  // Calculate something in the background.
  std::thread t{[&] { value = 42; }};

  // Use the calculated value.
  std::cerr << "value: " << value << std::endl; // 0 or 42?

  t.join();
}
