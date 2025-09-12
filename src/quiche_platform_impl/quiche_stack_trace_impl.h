#ifndef QUICHE_PLATFORM_IMPL_QUICHE_STACK_TRACE_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_STACK_TRACE_IMPL_H_

#include <string>
#include <vector>
#include "absl/types/span.h"

namespace quiche {

// 栈跟踪实现的存根
inline std::vector<void*> CurrentStackTraceImpl() {
    // 返回空的栈跟踪 - 在实际应用中这里会获取真实的栈信息
    return std::vector<void*>();
}

inline std::string SymbolizeStackTraceImpl(absl::Span<void* const> stacktrace) {
    // 返回简单的栈跟踪字符串 - 在实际应用中这里会符号化栈信息
    return "Stack trace not available in this implementation";
}

// 修复返回类型 - API期望返回字符串
inline std::string QuicheStackTraceImpl() {
    return "Stack trace not available in this implementation";
}

inline bool QuicheShouldRunStackTraceTestImpl() {
    // 在测试环境中返回false，避免运行栈跟踪测试
    return false;
}

}  // namespace quiche

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_STACK_TRACE_IMPL_H_