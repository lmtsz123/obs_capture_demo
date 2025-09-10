#include "quiche_time_utils_impl.h"
#include <chrono>

namespace quiche {

int64_t QuicheGetCurrentTimeInUsecImpl() {
    auto now = std::chrono::steady_clock::now();
    auto duration = now.time_since_epoch();
    return std::chrono::duration_cast<std::chrono::microseconds>(duration).count();
}

} // namespace quiche