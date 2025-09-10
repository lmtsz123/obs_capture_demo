#ifndef QUICHE_PLATFORM_IMPL_QUICHE_EXPORT_IMPL_H_
#define QUICHE_PLATFORM_IMPL_QUICHE_EXPORT_IMPL_H_

// QUICHE export macros implementation for Windows
// 这些宏用于控制符号的导出/导入

#ifdef _WIN32
  #ifdef QUICHE_IMPLEMENTATION
    #define QUICHE_EXPORT_IMPL __declspec(dllexport)
  #else
    #define QUICHE_EXPORT_IMPL __declspec(dllimport)
  #endif
  #define QUICHE_EXPORT_PRIVATE_IMPL
  #define QUICHE_NO_EXPORT_IMPL
#else
  // Linux/Unix platforms
  #if defined(__GNUC__) && __GNUC__ >= 4
    #define QUICHE_EXPORT_IMPL __attribute__((visibility("default")))
    #define QUICHE_EXPORT_PRIVATE_IMPL __attribute__((visibility("hidden")))
    #define QUICHE_NO_EXPORT_IMPL __attribute__((visibility("hidden")))
  #else
    #define QUICHE_EXPORT_IMPL
    #define QUICHE_EXPORT_PRIVATE_IMPL
    #define QUICHE_NO_EXPORT_IMPL
  #endif
#endif

// For static linking, disable all export macros
#ifdef QUICHE_STATIC_DEFINE
  #undef QUICHE_EXPORT_IMPL
  #define QUICHE_EXPORT_IMPL
  #undef QUICHE_EXPORT_PRIVATE_IMPL
  #define QUICHE_EXPORT_PRIVATE_IMPL
  #undef QUICHE_NO_EXPORT_IMPL
  #define QUICHE_NO_EXPORT_IMPL
#endif

#endif  // QUICHE_PLATFORM_IMPL_QUICHE_EXPORT_IMPL_H_
