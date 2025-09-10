#include "quiche_logging_impl.h"
#include <iostream>
#include <cstdlib>

namespace quiche {

QuicheLogMessage::QuicheLogMessage(const char* filename, int line, QuicheLogLevel level)
    : filename_(filename), line_(line), level_(level) {
}

QuicheLogMessage::~QuicheLogMessage() {
    const char* level_str = "UNKNOWN";
    switch (level_) {
        case QUICHE_LOG_INFO:
            level_str = "INFO";
            break;
        case QUICHE_LOG_WARNING:
            level_str = "WARNING";
            break;
        case QUICHE_LOG_ERROR:
            level_str = "ERROR";
            break;
        case QUICHE_LOG_FATAL:
            level_str = "FATAL";
            break;
    }
    
    std::cerr << "[" << level_str << "] " 
              << filename_ << ":" << line_ 
              << " " << stream_.str() << std::endl;
    
    if (level_ == QUICHE_LOG_FATAL) {
        std::abort();
    }
}

} // namespace quiche