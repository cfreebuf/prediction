// CopyRight 2019 360. All rights reserved.
// File   spin_mutex.h
// Date   2019-10-28 16:54:10
// Brief

#ifndef PREDICTION_UTIL_SPIN_MUTEX_H_
#define PREDICTION_UTIL_SPIN_MUTEX_H_

// Brief: Simple Implemention Of Spin Lock
// TODO(lidongming): need more performance test!
// WARN:cpu load!

#include <atomic>
#include <thread>
#include <mutex>
#include <iostream>

namespace prediction {
namespace util {

class SpinMutex {
 public:
  SpinMutex() = default;
  SpinMutex(const SpinMutex&) = delete;
  SpinMutex& operator= (const SpinMutex&) = delete;

  // Use std::lock_guard to implement RAII
  // Example:
  //  std::lock_guard<SpinMutex> lock(mutex_a);
  //  do something...
  void lock() {
      // Set Memory Order
      while(flag.test_and_set(std::memory_order_acquire));
  }
  void unlock() {
      flag.clear(std::memory_order_release);
  }

private:
  // atomic_flag is lock-free
  std::atomic_flag flag = ATOMIC_FLAG_INIT;
};

// typedef std::lock_guard<SpinMutex> ReadLock;
// typedef std::lock_guard<SpinMutex> WriteLock;

}  // namespace util
}  // namespace prediction

#endif  // PREDICTION_UTIL_SPIN_MUTEX_H_
