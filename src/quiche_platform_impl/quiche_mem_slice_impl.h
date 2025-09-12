#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_MEM_SLICE_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_MEM_SLICE_IMPL_H_

#include <memory>
#include <vector>
#include <cstring>

namespace quiche {

// Simple memory slice implementation
class QuicheMemSliceImpl {
 public:
  QuicheMemSliceImpl() : data_(nullptr), length_(0) {}
  
  QuicheMemSliceImpl(const char* data, size_t length) 
    : data_(std::make_unique<char[]>(length)), length_(length) {
    if (data && length > 0) {
      std::memcpy(data_.get(), data, length);
    }
  }
  
  QuicheMemSliceImpl(std::unique_ptr<char[]> data, size_t length)
    : data_(std::move(data)), length_(length) {}
  
  QuicheMemSliceImpl(QuicheMemSliceImpl&& other) noexcept
    : data_(std::move(other.data_)), length_(other.length_) {
    other.length_ = 0;
  }
  
  QuicheMemSliceImpl& operator=(QuicheMemSliceImpl&& other) noexcept {
    if (this != &other) {
      data_ = std::move(other.data_);
      length_ = other.length_;
      other.length_ = 0;
    }
    return *this;
  }
  
  // Non-copyable
  QuicheMemSliceImpl(const QuicheMemSliceImpl&) = delete;
  QuicheMemSliceImpl& operator=(const QuicheMemSliceImpl&) = delete;
  
  const char* data() const { return data_.get(); }
  size_t length() const { return length_; }
  bool empty() const { return length_ == 0; }
  
 private:
  std::unique_ptr<char[]> data_;
  size_t length_;
};

// Memory slice span implementation
class QuicheMemSliceSpanImpl {
 public:
  QuicheMemSliceSpanImpl() = default;
  
  QuicheMemSliceSpanImpl(std::vector<QuicheMemSliceImpl> slices)
    : slices_(std::move(slices)) {}
  
  QuicheMemSliceSpanImpl(QuicheMemSliceSpanImpl&& other) noexcept
    : slices_(std::move(other.slices_)) {}
  
  QuicheMemSliceSpanImpl& operator=(QuicheMemSliceSpanImpl&& other) noexcept {
    if (this != &other) {
      slices_ = std::move(other.slices_);
    }
    return *this;
  }
  
  // Non-copyable
  QuicheMemSliceSpanImpl(const QuicheMemSliceSpanImpl&) = delete;
  QuicheMemSliceSpanImpl& operator=(const QuicheMemSliceSpanImpl&) = delete;
  
  size_t NumSlices() const { return slices_.size(); }
  bool empty() const { return slices_.empty(); }
  
  const QuicheMemSliceImpl& GetSlice(size_t index) const {
    return slices_[index];
  }
  
  size_t total_length() const {
    size_t total = 0;
    for (const auto& slice : slices_) {
      total += slice.length();
    }
    return total;
  }
  
 private:
  std::vector<QuicheMemSliceImpl> slices_;
};

// Buffer allocator implementation
class QuicheBufferAllocatorImpl {
 public:
  static std::unique_ptr<char[]> Allocate(size_t size) {
    return std::make_unique<char[]>(size);
  }
  
  static QuicheMemSliceImpl NewSlice(size_t length) {
    auto data = Allocate(length);
    return QuicheMemSliceImpl(std::move(data), length);
  }
  
  static QuicheMemSliceImpl NewSlice(const char* data, size_t length) {
    return QuicheMemSliceImpl(data, length);
  }
};

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_MEM_SLICE_IMPL_H_