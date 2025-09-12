#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_THREAD_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_THREAD_IMPL_H_

#include <thread>
#include <mutex>
#include <condition_variable>

namespace quiche {

// Simple thread implementation using std::thread
class QuicheThreadImpl {
 public:
  template<typename Function, typename... Args>
  QuicheThreadImpl(Function&& f, Args&&... args) 
    : thread_(std::forward<Function>(f), std::forward<Args>(args)...) {}
  
  ~QuicheThreadImpl() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }
  
  void Join() {
    if (thread_.joinable()) {
      thread_.join();
    }
  }
  
  void Detach() {
    thread_.detach();
  }
  
 private:
  std::thread thread_;
};

// Mutex implementation
using QuicheMutexImpl = std::mutex;
using QuicheLockImpl = std::lock_guard<std::mutex>;

// Condition variable implementation  
using QuicheConditionVariableImpl = std::condition_variable;

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_THREAD_IMPL_H_