#include "quiche_flags_impl.h"

// 定义协议标志变量
#define QUICHE_PROTOCOL_FLAG(type, flag, value, ...) \
  type FLAGS_##flag = value;
#include "quiche_protocol_flags_list.h"
#undef QUICHE_PROTOCOL_FLAG

// 定义特性标志变量
#define QUICHE_FLAG(type, flag, internal_value, external_value, doc) \
  type FLAGS_##flag = external_value;
#include "quiche_feature_flags_list.h"
#undef QUICHE_FLAG