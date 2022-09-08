#pragma once

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <cstddef>
#include <future>
#include <ios>
#include <iostream>
#include <iterator>
#include <mutex>
#include <queue>
#include <thread>
#include <vector>

namespace lum {

struct mutator {
  /// Initialize next pass.
  void next() {
    std::next_permutation(std::begin(_permutation), std::end(_permutation));
    expected_thread.it = std::begin(_permutation);

    std::cerr << "next permutation: ";
    for (const auto id : _permutation) {
      std::cerr << id << " ";
    }
    std::cerr << std::endl;
  }

  void wait() {
    if (characterizing) {
      _threads.push_back(std::this_thread::get_id());
      std::cerr << "characterization phase, skipping waiting" << std::endl;
      return;
    }

    // Wait until our thread is free to acquire the lock.
    std::unique_lock<std::mutex> lock{expected_thread.mutex};
    assert(expected_thread.it < _permutation.end());
    expected_thread.cond.wait(lock, [this]() {
      const auto its_turn = expected_thread.is_current();
      std::cerr << std::this_thread::get_id()
                << " woken up, is it its turn: " << std::boolalpha << its_turn
                << std::endl;
      return its_turn;
    });

    // Update internals for the next call.
    std::cerr <<  "moving to next thread" << std::endl;
    expected_thread.it++;
    expected_thread.cond.notify_all();
  }

  // Start the mutation pass.
  void mutation_pass() {
    characterizing = false;
    _permutation = _threads;
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
  // Order recorded on characterization pass.
  std::vector<std::thread::id> _threads;

  // Permutation of the characterized order for the current pass.
  std::vector<std::thread::id> _permutation;

  // Threads that aren't expected to acquire the lock, will wait for this
  // future.
  struct {
    bool is_current() const { return std::this_thread::get_id() == *it; }

    std::mutex mutex;
    std::condition_variable cond;
    std::vector<std::thread::id>::iterator it;
  } expected_thread;
};

struct mutex {
  mutex(mutator &mut) : _mutator(mut) {}

  void lock() {
    _mutator.wait();
    std::cerr << std::this_thread::get_id() << " trying to lock" << std::endl;
    _real.lock();
    std::cerr << std::this_thread::get_id() << " locked" << std::endl;
  }

  void unlock() {
    _real.unlock();
    std::cerr << std::this_thread::get_id() << " unlocked" << std::endl;
  }

private:
  mutator &_mutator;
  std::mutex _real;
};

} // namespace lum
