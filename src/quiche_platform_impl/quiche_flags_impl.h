#ifndef QUICHE_PLATFORM_IMPL_QUICHE_FLAGS_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_FLAGS_IMPL_H_

#include <cstdint>
#include <string>

// 定义协议标志变量
#define QUICHE_PROTOCOL_FLAG(type, flag, ...) \
  extern type FLAGS_##flag;
#include "quiche_protocol_flags_list.h"
#undef QUICHE_PROTOCOL_FLAG

// 定义特性标志变量
#define QUICHE_FLAG(type, flag, internal_value, external_value, doc) \
  extern type FLAGS_##flag;
#include "quiche_feature_flags_list.h"
#undef QUICHE_FLAG

// 标志访问函数
#define GetQuicheFlagImpl(flag) GetQuicheFlagImplImpl(FLAGS_##flag)
inline bool GetQuicheFlagImplImpl(bool flag) { return flag; }
inline int32_t GetQuicheFlagImplImpl(int32_t flag) { return flag; }
inline int64_t GetQuicheFlagImplImpl(int64_t flag) { return flag; }
inline uint32_t GetQuicheFlagImplImpl(uint32_t flag) { return flag; }
inline uint64_t GetQuicheFlagImplImpl(uint64_t flag) { return flag; }
inline double GetQuicheFlagImplImpl(double flag) { return flag; }
inline std::string GetQuicheFlagImplImpl(const std::string& flag) {
  return flag;
}
#define SetQuicheFlagImpl(flag, value) ((FLAGS_##flag) = (value))

// 特性标志实现
#define QUICHE_RELOADABLE_FLAG(flag) quiche_reloadable_flag_##flag
#define QUICHE_RESTART_FLAG(flag) quiche_restart_flag_##flag
#define GetQuicheReloadableFlagImpl(flag) \
  GetQuicheFlag(QUICHE_RELOADABLE_FLAG(flag))
#define SetQuicheReloadableFlagImpl(flag, value) \
  SetQuicheFlag(QUICHE_RELOADABLE_FLAG(flag), value)
#define GetQuicheRestartFlagImpl(flag) GetQuicheFlag(QUICHE_RESTART_FLAG(flag))
#define SetQuicheRestartFlagImpl(flag, value) \
  SetQuicheFlag(QUICHE_RESTART_FLAG(flag), value)

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_FLAGS_IMPL_H_