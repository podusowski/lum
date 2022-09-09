#pragma once

#include "log.hpp"

#include <algorithm>
#include <cassert>
#include <condition_variable>
#include <future>
#include <ios>
#include <iostream>
#include <mutex>
#include <thread>
#include <vector>

namespace lum {

struct mutator {
  // Initialize next pass. Returns `true` as long as next pass hasn't been used
  // yet.
  bool next() {
    try_finishing_characterization_pass();

    // Note that when characterizing phase have just ended, this will
    // immediately switch to the next permutation. This means that for two locks
    // we only need two passes, e.g.:
    //  1. characterization pass: A locks, B locks
    //  2. mutation pass: B locks, A locks
    std::next_permutation(std::begin(permutation), std::end(permutation));
    _next.id = std::begin(permutation);

    log::trace{} << "next permutation prepared:";
    for (const auto id : permutation) {
      log::trace{} << "  " << id;
    }
    log::trace{};

    log::trace{} << "is same: " << std::boolalpha << (permutation == recorded);
    return !(permutation == recorded);
  }

  void wait() {
    if (characterizing) {
      recorded.push_back(std::this_thread::get_id());
      log::trace{} << "characterization phase, skipping waiting";
      return;
    }

    // Wait until our thread is free to acquire the lock.
    std::unique_lock<std::mutex> lock{_next.m};
    assert(_next.id < permutation.end());
    _next.cv.wait(lock, [this]() {
      const auto its_turn = _next.allowed();
      log::trace{} << "woken up, is it its turn: " << std::boolalpha
                   << its_turn;
      return its_turn;
    });
  }

  void unlock() {
    if (characterizing)
      return;
    std::unique_lock<std::mutex> lock{_next.m};
    log::trace{} << "moving to the next thread";
    _next.id++;
    _next.cv.notify_all();
  }

  ~mutator() {
    log::trace{} << "recorded locking profile:";
    for (auto id : recorded) {
      log::trace{} << "locked by " << id;
    }
  }

private:
  void try_finishing_characterization_pass() {
    if (!characterizing)
      return;

    characterizing = false;
    permutation = recorded;

    log::trace{} << "characterization done, " << recorded.size()
                 << " locks recorded:";
    for (const auto id : permutation) {
      log::trace{} << "  " << id;
    }
  }

  // Just record the order in which threads locks the mutex.
  bool characterizing = true;

  // Order recorded during characterization pass.
  std::vector<std::thread::id> recorded;

  // Current permutation of the characterized order for the current pass.
  std::vector<std::thread::id> permutation;

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
    log::trace{} << "trying to lock";
    _real.lock();
    log::trace{} << "locked";
  }

  void unlock() {
    _mutator.unlock();
    _real.unlock();
    log::trace{} << "unlocked";
  }

private:
  mutator &_mutator;
  std::mutex _real;
};

} // namespace lum
