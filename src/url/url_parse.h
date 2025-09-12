#ifndef URL_URL_PARSE_H_
#define URL_URL_PARSE_H_

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
void ParseAuthority(const char* spec,
                   const Component& authority,
                   Component* username,
                   Component* password,
                   Component* hostname,
                   Component* port_num);

// Stub implementation for ParsePort
int ParsePort(const char* spec, const Component& component);

}  // namespace url

#endif  // URL_URL_PARSE_H_