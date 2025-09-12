#ifndef QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_

#include <iostream>
#include <sstream>

// 简化的日志实现，兼容QUICHE的Abseil日志系统

// 统一的日志流类，根据是否启用来决定行为
class QuicheLogStream {
public:
    QuicheLogStream(const char* severity, bool enabled = true) 
        : severity_(severity), enabled_(enabled) {}
    
    ~QuicheLogStream() {
        if (enabled_ && !message_.empty()) {
            std::cerr << "[" << severity_ << "] " << message_ << std::endl;
            if (severity_ == "FATAL") {
                std::abort();
            }
        }
    }
    
    template<typename T>
    QuicheLogStream& operator<<(const T& value) {
        if (enabled_) {
            std::ostringstream oss;
            oss << value;
            message_ += oss.str();
        }
        return *this;
    }
    
    // 特殊类型的操作符重载
    QuicheLogStream& operator<<(const std::string& value) {
        if (enabled_) {
            message_ += value;
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(const char* value) {
        if (enabled_) {
            message_ += value;
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(char value) {
        if (enabled_) {
            message_ += value;
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(int value) {
        if (enabled_) {
            message_ += std::to_string(value);
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(long value) {
        if (enabled_) {
            message_ += std::to_string(value);
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(unsigned int value) {
        if (enabled_) {
            message_ += std::to_string(value);
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(unsigned long value) {
        if (enabled_) {
            message_ += std::to_string(value);
        }
        return *this;
    }
    
    QuicheLogStream& operator<<(bool value) {
        if (enabled_) {
            message_ += (value ? "true" : "false");
        }
        return *this;
    }
    
    // 支持流操纵符
    QuicheLogStream& operator<<(std::ostream& (*manip)(std::ostream&)) {
        // 对于流操纵符，我们简单忽略
        return *this;
    }
    
private:
    std::string message_;
    const char* severity_;
    bool enabled_;
};

// 为了兼容性，保留QuicheNullLogStream作为QuicheLogStream的别名
using QuicheNullLogStream = QuicheLogStream;

// 基础日志宏 - 模拟Abseil的ABSL_LOG
#define QUICHE_LOG_IMPL(severity) QuicheLogStream(#severity)
#define QUICHE_LOG_IMPL_FATAL() QuicheLogStream("FATAL")
#define QUICHE_LOG_IMPL_ERROR() QuicheLogStream("ERROR") 
#define QUICHE_LOG_IMPL_WARNING() QuicheLogStream("WARNING")
#define QUICHE_LOG_IMPL_INFO() QuicheLogStream("INFO")

// PLOG宏（带系统错误信息）
#define QUICHE_PLOG_IMPL(severity) QUICHE_LOG_IMPL(severity)
#define QUICHE_PLOG_IMPL_FATAL() QUICHE_LOG_IMPL_FATAL()
#define QUICHE_PLOG_IMPL_ERROR() QUICHE_LOG_IMPL_ERROR()
#define QUICHE_PLOG_IMPL_WARNING() QUICHE_LOG_IMPL_WARNING()
#define QUICHE_PLOG_IMPL_INFO() QUICHE_LOG_IMPL_INFO()

// DLOG宏（调试日志）
#ifdef NDEBUG
#define QUICHE_DLOG_IMPL(severity) QuicheLogStream(#severity, false)
#define QUICHE_DLOG_IMPL_FATAL() QuicheLogStream("FATAL", false)
#define QUICHE_DLOG_IMPL_ERROR() QuicheLogStream("ERROR", false)
#define QUICHE_DLOG_IMPL_WARNING() QuicheLogStream("WARNING", false)
#define QUICHE_DLOG_IMPL_INFO() QuicheLogStream("INFO", false)
#else
#define QUICHE_DLOG_IMPL(severity) QUICHE_LOG_IMPL(severity)
#define QUICHE_DLOG_IMPL_FATAL() QUICHE_LOG_IMPL_FATAL()
#define QUICHE_DLOG_IMPL_ERROR() QUICHE_LOG_IMPL_ERROR()
#define QUICHE_DLOG_IMPL_WARNING() QUICHE_LOG_IMPL_WARNING()
#define QUICHE_DLOG_IMPL_INFO() QUICHE_LOG_IMPL_INFO()
#endif

// 条件日志宏
#define QUICHE_LOG_IF_IMPL(severity, condition) \
    QuicheLogStream(#severity, (condition))

#define QUICHE_LOG_IF_IMPL_FATAL(condition) \
    QuicheLogStream("FATAL", (condition))
#define QUICHE_LOG_IF_IMPL_ERROR(condition) \
    QuicheLogStream("ERROR", (condition))
#define QUICHE_LOG_IF_IMPL_WARNING(condition) \
    QuicheLogStream("WARNING", (condition))
#define QUICHE_LOG_IF_IMPL_INFO(condition) \
    QuicheLogStream("INFO", (condition))

#define QUICHE_PLOG_IF_IMPL(severity, condition) QUICHE_LOG_IF_IMPL(severity, condition)
#define QUICHE_PLOG_IF_IMPL_FATAL(condition) QUICHE_LOG_IF_IMPL_FATAL(condition)
#define QUICHE_PLOG_IF_IMPL_ERROR(condition) QUICHE_LOG_IF_IMPL_ERROR(condition)
#define QUICHE_PLOG_IF_IMPL_WARNING(condition) QUICHE_LOG_IF_IMPL_WARNING(condition)
#define QUICHE_PLOG_IF_IMPL_INFO(condition) QUICHE_LOG_IF_IMPL_INFO(condition)

#ifdef NDEBUG
#define QUICHE_DLOG_IF_IMPL(severity, condition) \
    QuicheLogStream(#severity, false)
#define QUICHE_DLOG_IF_IMPL_FATAL(condition) \
    QuicheLogStream("FATAL", false)
#define QUICHE_DLOG_IF_IMPL_ERROR(condition) \
    QuicheLogStream("ERROR", false)
#define QUICHE_DLOG_IF_IMPL_WARNING(condition) \
    QuicheLogStream("WARNING", false)
#define QUICHE_DLOG_IF_IMPL_INFO(condition) \
    QuicheLogStream("INFO", false)
#else
#define QUICHE_DLOG_IF_IMPL(severity, condition) \
    QuicheLogStream(#severity, (condition))
#define QUICHE_DLOG_IF_IMPL_FATAL(condition) \
    QuicheLogStream("FATAL", (condition))
#define QUICHE_DLOG_IF_IMPL_ERROR(condition) \
    QuicheLogStream("ERROR", (condition))
#define QUICHE_DLOG_IF_IMPL_WARNING(condition) \
    QuicheLogStream("WARNING", (condition))
#define QUICHE_DLOG_IF_IMPL_INFO(condition) \
    QuicheLogStream("INFO", (condition))
#endif

// FIRST_N和EVERY_N_SEC宏（简化实现）
#define QUICHE_LOG_FIRST_N_IMPL(severity, n) QUICHE_LOG_IMPL(severity)
#define QUICHE_LOG_FIRST_N_IMPL_FATAL(n) QUICHE_LOG_IMPL_FATAL()
#define QUICHE_LOG_FIRST_N_IMPL_ERROR(n) QUICHE_LOG_IMPL_ERROR()
#define QUICHE_LOG_FIRST_N_IMPL_WARNING(n) QUICHE_LOG_IMPL_WARNING()
#define QUICHE_LOG_FIRST_N_IMPL_INFO(n) QUICHE_LOG_IMPL_INFO()

#define QUICHE_LOG_EVERY_N_SEC_IMPL(severity, seconds) QUICHE_LOG_IMPL(severity)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_FATAL(seconds) QUICHE_LOG_IMPL_FATAL()
#define QUICHE_LOG_EVERY_N_SEC_IMPL_ERROR(seconds) QUICHE_LOG_IMPL_ERROR()
#define QUICHE_LOG_EVERY_N_SEC_IMPL_WARNING(seconds) QUICHE_LOG_IMPL_WARNING()
#define QUICHE_LOG_EVERY_N_SEC_IMPL_INFO(seconds) QUICHE_LOG_IMPL_INFO()

// DFATAL宏
#ifdef NDEBUG
#define QUICHE_LOG_IMPL_DFATAL() QUICHE_LOG_IMPL_ERROR()
#define QUICHE_PLOG_IMPL_DFATAL() QUICHE_PLOG_IMPL_ERROR()
#define QUICHE_DLOG_IMPL_DFATAL() QUICHE_DLOG_IMPL_ERROR()
#define QUICHE_LOG_IF_IMPL_DFATAL(condition) QUICHE_LOG_IF_IMPL_ERROR(condition)
#define QUICHE_PLOG_IF_IMPL_DFATAL(condition) QUICHE_PLOG_IF_IMPL_ERROR(condition)
#define QUICHE_DLOG_IF_IMPL_DFATAL(condition) QUICHE_DLOG_IF_IMPL_ERROR(condition)
#define QUICHE_LOG_FIRST_N_IMPL_DFATAL(n) QUICHE_LOG_FIRST_N_IMPL_ERROR(n)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_DFATAL(seconds) QUICHE_LOG_EVERY_N_SEC_IMPL_ERROR(seconds)
#else
#define QUICHE_LOG_IMPL_DFATAL() QUICHE_LOG_IMPL_FATAL()
#define QUICHE_PLOG_IMPL_DFATAL() QUICHE_PLOG_IMPL_FATAL()
#define QUICHE_DLOG_IMPL_DFATAL() QUICHE_DLOG_IMPL_FATAL()
#define QUICHE_LOG_IF_IMPL_DFATAL(condition) QUICHE_LOG_IF_IMPL_FATAL(condition)
#define QUICHE_PLOG_IF_IMPL_DFATAL(condition) QUICHE_PLOG_IF_IMPL_FATAL(condition)
#define QUICHE_DLOG_IF_IMPL_DFATAL(condition) QUICHE_DLOG_IF_IMPL_FATAL(condition)
#define QUICHE_LOG_FIRST_N_IMPL_DFATAL(n) QUICHE_LOG_FIRST_N_IMPL_FATAL(n)
#define QUICHE_LOG_EVERY_N_SEC_IMPL_DFATAL(seconds) QUICHE_LOG_EVERY_N_SEC_IMPL_FATAL(seconds)
#endif

// VLOG和DVLOG宏
extern int FLAGS_v;  // 详细日志级别标志

#define QUICHE_VLOG_PREDICATE(verbose_level) (verbose_level <= FLAGS_v)

#define QUICHE_VLOG_IMPL(verbose_level) \
    QUICHE_LOG_IF_IMPL(INFO, QUICHE_VLOG_PREDICATE(verbose_level))
#define QUICHE_VLOG_IF_IMPL(verbose_level, condition) \
    QUICHE_LOG_IF_IMPL(INFO, (QUICHE_VLOG_PREDICATE(verbose_level) && (condition)))
#define QUICHE_DVLOG_IMPL(verbose_level) \
    QUICHE_DLOG_IF_IMPL(INFO, QUICHE_VLOG_PREDICATE(verbose_level))
#define QUICHE_DVLOG_IF_IMPL(verbose_level, condition) \
    QUICHE_DLOG_IF_IMPL(INFO, (QUICHE_VLOG_PREDICATE(verbose_level) && (condition)))

// 日志级别检查宏
#define QUICHE_LOG_INFO_IS_ON_IMPL() 1
#define QUICHE_LOG_WARNING_IS_ON_IMPL() 1
#define QUICHE_LOG_ERROR_IS_ON_IMPL() 1

#ifdef NDEBUG
#define QUICHE_DLOG_INFO_IS_ON_IMPL() 0
#else
#define QUICHE_DLOG_INFO_IS_ON_IMPL() 1
#endif

// CHECK宏（简化实现）
#define QUICHE_CHECK_IMPL(condition) \
    QuicheLogStream("FATAL", !(condition)) << "Check failed: " #condition " "

#define QUICHE_CHECK_EQ_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) == (val2))) << "Check failed: " #val1 " == " #val2 " "
#define QUICHE_CHECK_NE_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) != (val2))) << "Check failed: " #val1 " != " #val2 " "
#define QUICHE_CHECK_LE_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) <= (val2))) << "Check failed: " #val1 " <= " #val2 " "
#define QUICHE_CHECK_LT_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) < (val2))) << "Check failed: " #val1 " < " #val2 " "
#define QUICHE_CHECK_GE_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) >= (val2))) << "Check failed: " #val1 " >= " #val2 " "
#define QUICHE_CHECK_GT_IMPL(val1, val2) \
    QuicheLogStream("FATAL", !((val1) > (val2))) << "Check failed: " #val1 " > " #val2 " "

#define QUICHE_CHECK_OK_IMPL(condition) QUICHE_CHECK_IMPL(condition)

// DCHECK宏（调试版本的CHECK）
#ifdef NDEBUG
#define QUICHE_DCHECK_IMPL(condition) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_EQ_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_NE_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_LE_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_LT_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_GE_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#define QUICHE_DCHECK_GT_IMPL(val1, val2) QuicheLogStream("FATAL", false)
#else
#define QUICHE_DCHECK_IMPL(condition) QUICHE_CHECK_IMPL(condition)
#define QUICHE_DCHECK_EQ_IMPL(val1, val2) QUICHE_CHECK_EQ_IMPL(val1, val2)
#define QUICHE_DCHECK_NE_IMPL(val1, val2) QUICHE_CHECK_NE_IMPL(val1, val2)
#define QUICHE_DCHECK_LE_IMPL(val1, val2) QUICHE_CHECK_LE_IMPL(val1, val2)
#define QUICHE_DCHECK_LT_IMPL(val1, val2) QUICHE_CHECK_LT_IMPL(val1, val2)
#define QUICHE_DCHECK_GE_IMPL(val1, val2) QUICHE_CHECK_GE_IMPL(val1, val2)
#define QUICHE_DCHECK_GT_IMPL(val1, val2) QUICHE_CHECK_GT_IMPL(val1, val2)
#endif

#define QUICHE_NOTREACHED_IMPL() QUICHE_DCHECK_IMPL(false)

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_LOGGING_IMPL_H_