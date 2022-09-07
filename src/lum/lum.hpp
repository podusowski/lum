#pragma once

#include <cstddef>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace lum {

struct mutator {
  std::size_t acquire_id();

  void record_mutex_lock() {
    const auto id = std::this_thread::get_id();
    _threads.push_back(id);
  }

  // Just record order in which threads locks particular mutex.
  bool characterizing = true;

  ~mutator() {
    std::cerr << "recorded locking profile:" << std::endl;
    for (auto id : _threads) {
      std::cerr << "locked by " << id << std::endl;
    }
  }

private:
  std::size_t _id;
  // TODO: Only for a single mutex.
  std::vector<std::thread::id> _threads;
};

struct mutex {
  mutex(mutator &mut) : _mutator(mut) {}

  void lock() {
    if (_mutator.characterizing)
      _mutator.record_mutex_lock();
    std::cerr << "lock" << std::endl;
  }

  void unlock() { std::cerr << "unlock" << std::endl; }

private:
  mutator &_mutator;
  std::mutex _real;
};

} // namespace lum
