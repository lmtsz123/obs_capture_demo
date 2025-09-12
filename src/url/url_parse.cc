#include "url/url_parse.h"

namespace url {

void ParseAuthority(const char* spec,
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

int ParsePort(const char* spec, const Component& component) {
  if (!component.is_valid()) return -1;
  
  std::string port_str(spec + component.begin(), component.len());
  try {
    return std::stoi(port_str);
  } catch (...) {
    return -1;
  }
}

}  // namespace url