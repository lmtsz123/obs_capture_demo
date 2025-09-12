#ifndef QUICHE_COMMON_PLATFORM_IMPL_QUICHE_STRING_UTILS_IMPL_H_
#define QUICHE_COMMON_PLATFORM_IMPL_QUICHE_STRING_UTILS_IMPL_H_

#include <string>
#include <vector>
#include <sstream>
#include <algorithm>
#include <cctype>
#include <cstdarg>
#include <cstdio>

namespace quiche {

// String utility implementations
class QuicheStringUtilsImpl {
 public:
  static std::string QuicheStringPrintfImpl(const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    // Get required size
    int size = std::vsnprintf(nullptr, 0, format, args);
    va_end(args);
    
    if (size <= 0) return "";
    
    // Create string with required size
    std::string result(size, '\0');
    va_start(args, format);
    std::vsnprintf(&result[0], size + 1, format, args);
    va_end(args);
    
    return result;
  }
  
  static void QuicheStringAppendPrintfImpl(std::string* output, const char* format, ...) {
    va_list args;
    va_start(args, format);
    
    int size = std::vsnprintf(nullptr, 0, format, args);
    va_end(args);
    
    if (size <= 0) return;
    
    size_t old_size = output->size();
    output->resize(old_size + size);
    
    va_start(args, format);
    std::vsnprintf(&(*output)[old_size], size + 1, format, args);
    va_end(args);
  }
  
  static std::string QuicheStringToLowerImpl(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::tolower(c); });
    return result;
  }
  
  static std::string QuicheStringToUpperImpl(const std::string& input) {
    std::string result = input;
    std::transform(result.begin(), result.end(), result.begin(),
                   [](unsigned char c) { return std::toupper(c); });
    return result;
  }
  
  static std::vector<std::string> QuicheStringSplitImpl(const std::string& input, char delimiter) {
    std::vector<std::string> result;
    std::stringstream ss(input);
    std::string item;
    
    while (std::getline(ss, item, delimiter)) {
      result.push_back(item);
    }
    
    return result;
  }
  
  static std::string QuicheStringJoinImpl(const std::vector<std::string>& strings, const std::string& delimiter) {
    if (strings.empty()) return "";
    
    std::string result = strings[0];
    for (size_t i = 1; i < strings.size(); ++i) {
      result += delimiter + strings[i];
    }
    
    return result;
  }
  
  static bool QuicheStringStartsWithImpl(const std::string& input, const std::string& prefix) {
    return input.size() >= prefix.size() && 
           input.substr(0, prefix.size()) == prefix;
  }
  
  static bool QuicheStringEndsWithImpl(const std::string& input, const std::string& suffix) {
    return input.size() >= suffix.size() && 
           input.substr(input.size() - suffix.size()) == suffix;
  }
};

}  // namespace quiche

#endif  // QUICHE_COMMON_PLATFORM_IMPL_QUICHE_STRING_UTILS_IMPL_H_