#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TESTVALUE_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TESTVALUE_IMPL_H_

namespace quiche {

// Simple stub implementation for test value functionality
template<typename T>
class QuicheTestValueImpl {
 public:
  static T Get() {
    return T{};  // Return default value
  }
  
  static void Set(const T& value) {
    // Stub implementation - do nothing
  }
};

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_TESTVALUE_IMPL_H_