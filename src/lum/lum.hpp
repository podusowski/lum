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
    _next.id = std::begin(_permutation);

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
    std::unique_lock<std::mutex> lock{_next.m};
    assert(_next.id < _permutation.end());
    _next.cv.wait(lock, [this]() {
      const auto its_turn = _next.allowed();
      synced_cerr{} << std::this_thread::get_id()
                    << " woken up, is it its turn: " << std::boolalpha
                    << its_turn;
      return its_turn;
    });
  }

  void unlock() {
    if (characterizing)
      return;
    std::unique_lock<std::mutex> lock{_next.m};
    synced_cerr{} << "moving to the next thread";
    _next.id++;
    _next.cv.notify_all();
  }

  // Start the mutation pass.
  void mutation_pass() {
    characterizing = false;
    _permutation = _threads;
  }

  ~mutator() {
    synced_cerr{} << "recorded locking profile:";
    for (auto id : _threads) {
      synced_cerr{} << "locked by " << id;
    }
  }

private:
  // Just record the order in which threads locks the mutex.
  bool characterizing = true;

  // Order recorded on characterization pass.
  std::vector<std::thread::id> _threads;

  // Permutation of the characterized order for the current pass.
  std::vector<std::thread::id> _permutation;

  // Thread allowed to acquire the next lock.
  struct {
    bool allowed() const { return std::this_thread::get_id() == *id; }

    std::mutex m;
    std::condition_variable cv;
    std::vector<std::thread::id>::iterator id;
  } _next;
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
    _mutator.unlock();
    _real.unlock();
    synced_cerr{} << std::this_thread::get_id() << " unlocked";
  }

private:
  mutator &_mutator;
  std::mutex _real;
};

} // namespace lum
