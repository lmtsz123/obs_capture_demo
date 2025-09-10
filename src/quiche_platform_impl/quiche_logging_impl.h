#ifndef QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_

#include <iostream>
#include <sstream>

// 日志级别定义
enum QuicheLogLevel {
    QUICHE_LOG_INFO = 0,
    QUICHE_LOG_WARNING = 1,
    QUICHE_LOG_ERROR = 2,
    QUICHE_LOG_FATAL = 3
};

// 日志实现宏
#define QUICHE_LOG_IMPL(severity) \
    ::quiche::QuicheLogMessage(__FILE__, __LINE__, QUICHE_LOG_##severity).stream()

#define QUICHE_LOG_IF_IMPL(severity, condition) \
    !(condition) ? (void) 0 : ::quiche::QuicheLogMessageVoidify() & QUICHE_LOG_IMPL(severity)

#define QUICHE_VLOG_IMPL(verbose_level) \
    (verbose_level <= 1) ? QUICHE_LOG_IMPL(INFO) : ::quiche::QuicheNullStream().stream()

#define QUICHE_DVLOG_IMPL(verbose_level) QUICHE_VLOG_IMPL(verbose_level)
#define QUICHE_DVLOG_IF_IMPL(verbose_level, condition) QUICHE_LOG_IF_IMPL(INFO, condition)
#define QUICHE_DLOG_IMPL(severity) QUICHE_LOG_IMPL(severity)
#define QUICHE_DLOG_IF_IMPL(severity, condition) QUICHE_LOG_IF_IMPL(severity, condition)
#define QUICHE_LOG_FIRST_N_IMPL(severity, n) QUICHE_LOG_IMPL(severity)
#define QUICHE_LOG_EVERY_N_SEC_IMPL(severity, seconds) QUICHE_LOG_IMPL(severity)
#define QUICHE_PLOG_IMPL(severity) QUICHE_LOG_IMPL(severity)

#define QUICHE_NOTREACHED_IMPL() QUICHE_LOG_IMPL(FATAL) << "NOTREACHED() "

#define QUICHE_DLOG_INFO_IS_ON_IMPL() true
#define QUICHE_LOG_INFO_IS_ON_IMPL() true
#define QUICHE_LOG_WARNING_IS_ON_IMPL() true
#define QUICHE_LOG_ERROR_IS_ON_IMPL() true

// CHECK宏
#define QUICHE_CHECK_IMPL(condition) \
    !(condition) ? QUICHE_LOG_IMPL(FATAL) << "Check failed: " #condition " " : ::quiche::QuicheNullStream().stream()

#define QUICHE_CHECK_OK_IMPL(condition) QUICHE_CHECK_IMPL(condition)
#define QUICHE_CHECK_EQ_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) == (val2))
#define QUICHE_CHECK_NE_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) != (val2))
#define QUICHE_CHECK_LE_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) <= (val2))
#define QUICHE_CHECK_LT_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) < (val2))
#define QUICHE_CHECK_GE_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) >= (val2))
#define QUICHE_CHECK_GT_IMPL(val1, val2) QUICHE_CHECK_IMPL((val1) > (val2))

// DCHECK宏 - 支持流式输出
#define QUICHE_DCHECK_IMPL(condition) \
    !(condition) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_EQ_IMPL(val1, val2) \
    !((val1) == (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_NE_IMPL(val1, val2) \
    !((val1) != (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_LE_IMPL(val1, val2) \
    !((val1) <= (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_LT_IMPL(val1, val2) \
    !((val1) < (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_GE_IMPL(val1, val2) \
    !((val1) >= (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()
#define QUICHE_DCHECK_GT_IMPL(val1, val2) \
    !((val1) > (val2)) ? ::quiche::QuicheNullStream().stream() : ::quiche::QuicheNullStream().stream()

namespace quiche {

class QuicheLogMessage {
public:
    QuicheLogMessage(const char* filename, int line, QuicheLogLevel level);
    ~QuicheLogMessage();
    
    std::ostream& stream() { return stream_; }

private:
    const char* filename_;
    int line_;
    QuicheLogLevel level_;
    std::ostringstream stream_;
};

class QuicheLogMessageVoidify {
public:
    QuicheLogMessageVoidify() = default;
    void operator&(std::ostream&) {}
};

class QuicheNullStream {
public:
    std::ostream& stream() {
        static std::ostream null_stream(nullptr);
        return null_stream;
    }
};

} // namespace quiche

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_