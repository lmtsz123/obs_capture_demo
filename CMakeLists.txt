cmake_minimum_required(VERSION 3.15)

# vcpkg integration
if(DEFINED ENV{VCPKG_ROOT} AND NOT DEFINED CMAKE_TOOLCHAIN_FILE)
  set(CMAKE_TOOLCHAIN_FILE "$ENV{VCPKG_ROOT}/scripts/buildsystems/vcpkg.cmake"
      CACHE STRING "")
endif()

project(obs_capture_demo)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 全局禁用第三方库的install目标以避免导出冲突
set(CMAKE_SKIP_INSTALL_ALL_DEPENDENCY ON)
set(CMAKE_INSTALL_MESSAGE NEVER)

# 重写install命令以禁用第三方库的安装
function(install)
    # 空函数，禁用所有install调用
endfunction()

# 第三方依赖库配置
set(THIRD_PARTY_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third-party)

# Abseil C++
set(ABSL_PROPAGATE_CXX_STD ON)
set(ABSL_ENABLE_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/abseil-cpp EXCLUDE_FROM_ALL)

# zlib
add_subdirectory(${THIRD_PARTY_DIR}/zlib EXCLUDE_FROM_ALL)

# RE2
set(RE2_BUILD_TESTING OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/re2 EXCLUDE_FROM_ALL)

# Protobuf
set(protobuf_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(protobuf_MSVC_STATIC_RUNTIME OFF CACHE BOOL "" FORCE)
set(protobuf_INSTALL OFF CACHE BOOL "" FORCE)
add_subdirectory(${THIRD_PARTY_DIR}/protobuf EXCLUDE_FROM_ALL)

# BoringSSL配置 - 禁用汇编优化以避免NASM依赖
if(EXISTS ${THIRD_PARTY_DIR}/boringssl)
    # 禁用BoringSSL的汇编优化和安装
    set(OPENSSL_NO_ASM ON CACHE BOOL "" FORCE)
    set(BUILD_SHARED_LIBS OFF CACHE BOOL "" FORCE)
    add_subdirectory(${THIRD_PARTY_DIR}/boringssl EXCLUDE_FROM_ALL)
    set(SSL_LIBRARY ssl)
    set(CRYPTO_LIBRARY crypto)
else()
    # 如果BoringSSL不存在，使用系统OpenSSL
    find_package(OpenSSL REQUIRED)
    set(SSL_LIBRARY OpenSSL::SSL)
    set(CRYPTO_LIBRARY OpenSSL::Crypto)
endif()

# GoogleTest (可选，仅测试时需要)
option(BUILD_TESTS "Build tests" OFF)
if(BUILD_TESTS)
    add_subdirectory(${THIRD_PARTY_DIR}/googletest EXCLUDE_FROM_ALL)
endif()

# SDL2 - 使用third-party目录中的SDL2
set(SDL2_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third-party/SDL2-2.24.0)
set(SDL2_INCLUDE_DIR ${SDL2_ROOT_DIR}/include)

# 根据平台选择合适的库目录
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    # 64位
    set(SDL2_LIB_DIR ${SDL2_ROOT_DIR}/lib/x64)
else()
    # 32位
    set(SDL2_LIB_DIR ${SDL2_ROOT_DIR}/lib/x86)
endif()

find_library(SDL2_LIBRARY SDL2 PATHS ${SDL2_LIB_DIR} NO_DEFAULT_PATH)
find_library(SDL2MAIN_LIBRARY SDL2main PATHS ${SDL2_LIB_DIR} NO_DEFAULT_PATH)

# OpenGL
find_package(OpenGL REQUIRED)

# Vulkan (可选)
find_package(Vulkan QUIET)

# Windows Media Foundation、DirectShow 和 Direct3D 库
set(WINDOWS_LIBS
    mf
    mfplat
    mfreadwrite
    mfuuid
    strmiids
    ole32
    oleaut32
    uuid
    winmm
    d3d11
    dxgi
    d3dcompiler
)

# QUICHE配置
set(QUICHE_ROOT_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third-party/quiche)
set(QUICHE_INCLUDE_DIRS
    ${QUICHE_ROOT_DIR}
    ${QUICHE_ROOT_DIR}/quiche
    ${CMAKE_CURRENT_SOURCE_DIR}/src  # 为了找到quiche_platform_impl
)

# QUICHE核心源文件 (使用实际存在的文件)
set(QUICHE_SOURCES
    # 平台抽象层 (需要自己实现)
    src/quiche_platform_impl/quiche_bug_tracker_impl.cc
    src/quiche_platform_impl/quiche_flags_impl.cc
    src/quiche_platform_impl/quiche_logging_impl.cc
    src/quiche_platform_impl/quiche_time_utils_impl.cc
    
    # QUICHE核心文件 (使用实际存在的文件)
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_buffer_allocator.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_data_reader.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_data_writer.cc
    ${QUICHE_ROOT_DIR}/quiche/common/capsule.cc
    ${QUICHE_ROOT_DIR}/quiche/common/internet_checksum.cc
    ${QUICHE_ROOT_DIR}/quiche/common/bug_utils.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_crypto_logging.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_ip_address.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_ip_address_family.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_mem_slice.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_mem_slice_storage.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_random.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_simple_arena.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_socket_address.cc
    ${QUICHE_ROOT_DIR}/quiche/common/quiche_text_utils.cc
    ${QUICHE_ROOT_DIR}/quiche/common/simple_buffer_allocator.cc
    ${QUICHE_ROOT_DIR}/quiche/common/structured_headers.cc
    ${QUICHE_ROOT_DIR}/quiche/common/vectorized_io_utils.cc
)

# QUICHE静态链接定义
add_definitions(-DQUICHE_STATIC_DEFINE)

# Windows特定定义 - 禁用min/max宏以避免与std::min/std::max冲突
if(WIN32)
    add_definitions(-DNOMINMAX)
    add_definitions(-DWIN32_LEAN_AND_MEAN)
endif()

# 包含目录
include_directories(
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
    ${QUICHE_INCLUDE_DIRS}
    src/
)

# 如果找到Vulkan，添加其包含目录
if(Vulkan_FOUND)
    include_directories(${Vulkan_INCLUDE_DIRS})
    add_definitions(-DVULKAN_AVAILABLE)
endif()

# 源文件
set(SOURCES
    src/main.cpp
    src/camera_capture.cpp
    src/nv12_to_rgba_shader.cpp
    src/d3d11_renderer.cpp
    src/vulkan_renderer.cpp
    src/renderer_factory.cpp
    src/dshow_device_enum.cpp
    src/dshow_capture.cpp
    src/wmf_capture.cpp
    src/quic_test.cpp
    ${QUICHE_SOURCES}
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${WINDOWS_LIBS}
    
    # QUICHE依赖
    absl::base
    absl::strings
    absl::str_format
    absl::time
    absl::synchronization
    absl::flat_hash_map
    absl::flat_hash_set
    absl::btree
    absl::optional
    absl::variant
    protobuf::libprotobuf
    ${SSL_LIBRARY}
    ${CRYPTO_LIBRARY}
    re2::re2
    zlibstatic
)

# 如果找到Vulkan，链接Vulkan库
if(Vulkan_FOUND)
    target_link_libraries(${PROJECT_NAME} ${Vulkan_LIBRARIES})
endif()

# 复制SDL2 DLL到输出目录（Windows平台）
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()

# 编译Vulkan着色器
find_program(GLSLC glslc HINTS ${Vulkan_GLSLC_EXECUTABLE} PATHS ENV VULKAN_SDK PATH_SUFFIXES bin)

if(GLSLC)
    set(SHADER_SOURCE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/shaders)
    set(SHADER_BINARY_DIR ${CMAKE_CURRENT_BINARY_DIR}/shaders)
    
    file(MAKE_DIRECTORY ${SHADER_BINARY_DIR})
    
    # 编译顶点着色器
    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/vulkan_vertex_shader.spv
        COMMAND ${GLSLC} ${SHADER_SOURCE_DIR}/vulkan_vertex_shader.vert -o ${SHADER_BINARY_DIR}/vulkan_vertex_shader.spv
        DEPENDS ${SHADER_SOURCE_DIR}/vulkan_vertex_shader.vert
        COMMENT "Compiling Vulkan vertex shader"
    )
    
    # 编译片段着色器
    add_custom_command(
        OUTPUT ${SHADER_BINARY_DIR}/vulkan_fragment_shader.spv
        COMMAND ${GLSLC} ${SHADER_SOURCE_DIR}/vulkan_fragment_shader.frag -o ${SHADER_BINARY_DIR}/vulkan_fragment_shader.spv
        DEPENDS ${SHADER_SOURCE_DIR}/vulkan_fragment_shader.frag
        COMMENT "Compiling Vulkan fragment shader"
    )
    
    # 创建着色器目标
    add_custom_target(vulkan_shaders
        DEPENDS ${SHADER_BINARY_DIR}/vulkan_vertex_shader.spv ${SHADER_BINARY_DIR}/vulkan_fragment_shader.spv
    )
    
    # 让主目标依赖着色器编译
    add_dependencies(${PROJECT_NAME} vulkan_shaders)
    
    # 复制编译好的着色器到输出目录
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
        ${SHADER_BINARY_DIR}
        $<TARGET_FILE_DIR:${PROJECT_NAME}>/shaders)
else()
    message(WARNING "glslc not found. Vulkan shaders will not be compiled.")
endif()