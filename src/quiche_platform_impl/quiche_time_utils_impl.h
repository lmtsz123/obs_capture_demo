#ifndef QUICHE_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_

#include <cstdint>

namespace quiche {

int64_t QuicheGetCurrentTimeInUsecImpl();

} // namespace quiche

#define QUICHE_TIME_UTILS_IMPL() ::quiche::QuicheGetCurrentTimeInUsecImpl()

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_TIME_UTILS_IMPL_H_