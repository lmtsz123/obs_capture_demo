#ifndef QUICHE_PLATFORM_IMPL_QUICHE_BUG_TRACKER_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_BUG_TRACKER_IMPL_H_

#include <iostream>
#include <sstream>

namespace quiche {

// 用于支持流式输出的Bug跟踪器类
class QuicheBugTracker {
public:
    explicit QuicheBugTracker(const char* bug_id) : active_(true) {
        stream_ << "QUICHE BUG " << bug_id << ": ";
    }
    
    // 用于条件性Bug跟踪的构造函数
    QuicheBugTracker(const char* bug_id, bool condition) : active_(condition) {
        if (active_) {
            stream_ << "QUICHE BUG " << bug_id << ": ";
        }
    }
    
    ~QuicheBugTracker() {
        if (active_) {
            std::cerr << stream_.str() << std::endl;
        }
    }
    
    template<typename T>
    QuicheBugTracker& operator<<(const T& value) {
        if (active_) {
            stream_ << value;
        }
        return *this;
    }
    
private:
    std::ostringstream stream_;
    bool active_;
};

} // namespace quiche

#define QUICHE_BUG_IMPL(bug_id) ::quiche::QuicheBugTracker(#bug_id)
#define QUICHE_BUG_IF_IMPL(bug_id, condition) \
    ::quiche::QuicheBugTracker(#bug_id, (condition))
#define QUICHE_PEER_BUG_IMPL(bug_id) ::quiche::QuicheBugTracker(#bug_id)
#define QUICHE_PEER_BUG_IF_IMPL(bug_id, condition) \
    ::quiche::QuicheBugTracker(#bug_id, (condition))

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_BUG_TRACKER_IMPL_H_