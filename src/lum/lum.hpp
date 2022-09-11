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
    next_thread.id = std::begin(permutation);

    log::trace{} << "next permutation: " << log::range(permutation) << "\n";

    // Whether we haven't yet reached the point where we started.
    std::unique_lock<std::mutex> lock{recorded.m};
    return !(permutation == recorded.data);
  }

  void wait() {
    if (characterizing) {
      std::unique_lock<std::mutex> lock{recorded.m};
      recorded.data.push_back(std::this_thread::get_id());
      log::trace{} << "characterization phase, skipping waiting";
      return;
    }

    // Wait until our thread is free to acquire the lock.
    std::unique_lock<std::mutex> lock{next_thread.m};
    assert(next_thread.id < permutation.end());
    next_thread.cv.wait(lock, [this]() {
      const auto its_turn = next_thread.allowed();
      log::trace{} << "woken up, is it its turn: " << std::boolalpha
                   << its_turn;
      return its_turn;
    });
  }

  void unlock() {
    if (characterizing)
      return;
    std::unique_lock<std::mutex> lock{next_thread.m};
    log::trace{} << "moving to the next thread";
    next_thread.id++;
    next_thread.cv.notify_all();
  }

  ~mutator() {
    std::unique_lock<std::mutex> lock{recorded.m};
    log::trace{} << "recorded locking profile: " << log::range(recorded.data);
  }

private:
  void try_finishing_characterization_pass() {
    if (!characterizing)
      return;

    characterizing = false;
    std::unique_lock<std::mutex> lock{recorded.m};
    permutation = recorded.data;

    log::trace{} << "characterization done, " << recorded.data.size()
                 << " locks recorded: " << log::range(permutation);
  }

  // Just record the order in which threads locks the mutex.
  bool characterizing = true;

  // Order recorded during characterization pass.
  struct {
    std::mutex m;
    std::vector<std::thread::id> data;
  } recorded;

  // Current permutation of the characterized order for the current pass.
  std::vector<std::thread::id> permutation;

  // Thread allowed to acquire the next lock.
  struct {
    bool allowed() const { return std::this_thread::get_id() == *id; }

    std::mutex m;
    std::condition_variable cv;
    std::vector<std::thread::id>::iterator id;
  } next_thread;
};

inline mutator &global_mutator() {
  static mutator m;
  return m;
}

struct mutex {
  mutex(mutator &mut = global_mutator()) : _mutator(mut) {}

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
