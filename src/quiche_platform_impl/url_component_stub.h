#ifndef QUICHE_PLATFORM_IMPL_URL_COMPONENT_STUB_H_
#define QUICHE_PLATFORM_IMPL_URL_COMPONENT_STUB_H_

#include <string>

namespace url {

// Simple stub implementation for URL component parsing
class Component {
 public:
  Component() : begin_(0), len_(0) {}
  Component(int begin, int len) : begin_(begin), len_(len) {}
  
  bool is_valid() const { return len_ > 0; }
  bool is_nonempty() const { return len_ > 0; }
  
  int begin() const { return begin_; }
  int len() const { return len_; }
  int end() const { return begin_ + len_; }
  
 private:
  int begin_;
  int len_;
};

// Stub implementation for ParseAuthority
inline void ParseAuthority(const char* spec,
                          const Component& authority,
                          Component* username,
                          Component* password,
                          Component* hostname,
                          Component* port_num) {
  // Very basic parsing for "hostname:port" format
  std::string auth_str(spec + authority.begin(), authority.len());
  
  // Find colon separator
  size_t colon_pos = auth_str.find(':');
  
  if (username) *username = Component(); // Invalid by default
  if (password) *password = Component(); // Invalid by default
  
  if (colon_pos != std::string::npos) {
    // hostname:port format
    if (hostname) {
      *hostname = Component(authority.begin(), colon_pos);
    }
    if (port_num) {
      *port_num = Component(authority.begin() + colon_pos + 1, 
                           authority.len() - colon_pos - 1);
    }
  } else {
    // Just hostname
    if (hostname) {
      *hostname = authority;
    }
    if (port_num) {
      *port_num = Component(); // Invalid
    }
  }
}

// Stub implementation for ParsePort
inline int ParsePort(const char* spec, const Component& component) {
  if (!component.is_valid()) return -1;
  
  std::string port_str(spec + component.begin(), component.len());
  try {
    return std::stoi(port_str);
  } catch (...) {
    return -1;
  }
}

}  // namespace url

#endif  // QUICHE_PLATFORM_IMPL_URL_COMPONENT_STUB_H_