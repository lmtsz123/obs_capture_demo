#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_SERVER_STATS_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_SERVER_STATS_IMPL_H_

#include <string>

namespace quiche {

// Simple stub implementation for server stats
class QuicheServerStatsImpl {
 public:
  static void SetFlag(const std::string& flag_name, bool value) {
    // Stub implementation
  }
  
  static void SetFlag(const std::string& flag_name, int value) {
    // Stub implementation
  }
  
  static void SetFlag(const std::string& flag_name, const std::string& value) {
    // Stub implementation
  }
  
  static void IncrementCounter(const std::string& counter_name) {
    // Stub implementation
  }
};

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_SERVER_STATS_IMPL_H_