#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_REFERENCE_COUNTED_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_REFERENCE_COUNTED_IMPL_H_

#include <atomic>
#include <memory>

namespace quiche {

// Simple reference counted base class implementation
class QuicheReferenceCountedImpl {
 public:
  QuicheReferenceCountedImpl() : ref_count_(1) {}

 protected:
  virtual ~QuicheReferenceCountedImpl() = default;

 private:
  friend void QuicheReferenceCountedImplAddRef(QuicheReferenceCountedImpl* p);
  friend void QuicheReferenceCountedImplRelease(QuicheReferenceCountedImpl* p);
  
  std::atomic<int> ref_count_;
};

inline void QuicheReferenceCountedImplAddRef(QuicheReferenceCountedImpl* p) {
  if (p) {
    p->ref_count_.fetch_add(1, std::memory_order_relaxed);
  }
}

inline void QuicheReferenceCountedImplRelease(QuicheReferenceCountedImpl* p) {
  if (p && p->ref_count_.fetch_sub(1, std::memory_order_acq_rel) == 1) {
    delete p;
  }
}

// Reference counted pointer implementation
template <typename T>
class QuicheReferenceCountedPointerImpl {
 public:
  QuicheReferenceCountedPointerImpl() : ptr_(nullptr) {}
  
  explicit QuicheReferenceCountedPointerImpl(T* p) : ptr_(p) {
    // Don't add ref here - the raw pointer constructor assumes reference count is already 1
  }
  
  QuicheReferenceCountedPointerImpl(std::nullptr_t) : ptr_(nullptr) {}
  
  QuicheReferenceCountedPointerImpl(const QuicheReferenceCountedPointerImpl& other) 
      : ptr_(other.ptr_) {
    if (ptr_) {
      QuicheReferenceCountedImplAddRef(ptr_);
    }
  }
  
  QuicheReferenceCountedPointerImpl(QuicheReferenceCountedPointerImpl&& other) 
      : ptr_(other.ptr_) {
    other.ptr_ = nullptr;
  }
  
  ~QuicheReferenceCountedPointerImpl() {
    if (ptr_) {
      QuicheReferenceCountedImplRelease(ptr_);
    }
  }
  
  QuicheReferenceCountedPointerImpl& operator=(const QuicheReferenceCountedPointerImpl& other) {
    if (this != &other) {
      if (ptr_) {
        QuicheReferenceCountedImplRelease(ptr_);
      }
      ptr_ = other.ptr_;
      if (ptr_) {
        QuicheReferenceCountedImplAddRef(ptr_);
      }
    }
    return *this;
  }
  
  QuicheReferenceCountedPointerImpl& operator=(QuicheReferenceCountedPointerImpl&& other) {
    if (this != &other) {
      if (ptr_) {
        QuicheReferenceCountedImplRelease(ptr_);
      }
      ptr_ = other.ptr_;
      other.ptr_ = nullptr;
    }
    return *this;
  }
  
  T* get() const { return ptr_; }
  T& operator*() const { return *ptr_; }
  T* operator->() const { return ptr_; }
  
  explicit operator bool() const { return ptr_ != nullptr; }
  
 private:
  T* ptr_;
};

template <typename T, typename... Args>
QuicheReferenceCountedPointerImpl<T> QuicheMakeRefCountedImpl(Args&&... args) {
  return QuicheReferenceCountedPointerImpl<T>(new T(std::forward<Args>(args)...));
}

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_REFERENCE_COUNTED_IMPL_H_