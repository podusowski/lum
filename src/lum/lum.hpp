#pragma once

#include "log.hpp"

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

    synced_cerr{} << "next permutation: ";
    for (const auto id : _permutation) {
      synced_cerr{} << id << " ";
    }
    synced_cerr{};
  }

  void wait() {
    if (characterizing) {
      _threads.push_back(std::this_thread::get_id());
      synced_cerr{} << "characterization phase, skipping waiting";
      return;
    }

    // Wait until our thread is free to acquire the lock.
    std::unique_lock<std::mutex> lock{expected_thread.mutex};
    assert(expected_thread.it < _permutation.end());
    expected_thread.cond.wait(lock, [this]() {
      const auto its_turn = expected_thread.is_current();
      synced_cerr{} << std::this_thread::get_id()
                    << " woken up, is it its turn: " << std::boolalpha
                    << its_turn;
      return its_turn;
    });

    // Update internals for the next call.
    synced_cerr{} << "moving to next thread";
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
    synced_cerr{} << "recorded locking profile:";
    for (auto id : _threads) {
      synced_cerr{} << "locked by " << id;
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
    synced_cerr{} << std::this_thread::get_id() << " trying to lock";
    _real.lock();
    synced_cerr{} << std::this_thread::get_id() << " locked";
  }

  void unlock() {
    _real.unlock();
    synced_cerr{} << std::this_thread::get_id() << " unlocked";
  }

private:
  mutator &_mutator;
  std::mutex _real;
};

} // namespace lum
