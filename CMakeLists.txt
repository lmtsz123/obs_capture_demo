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

# 定义QUIC需要的编译标志
add_definitions(-DQUIC_FRAME_DEBUG=0)

# QUICHE核心源文件 (包含QUIC协议核心实现)
set(QUICHE_SOURCES
    # 平台抽象层 (需要自己实现)
    src/quiche_platform_impl/quiche_bug_tracker_impl.cc
    src/quiche_platform_impl/quiche_flags_impl.cc
    src/quiche_platform_impl/quiche_logging_impl.cc
    src/quiche_platform_impl/quiche_time_utils_impl.cc
    
    # QUICHE通用工具
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
    
    # QUIC核心协议实现
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_alarm.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_bandwidth.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_blocked_writer_list.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_buffered_packet_store.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_coalesced_packet.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_config.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection_alarms.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection_context.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection_id.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection_id_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_connection_stats.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_constants.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_control_frame_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_data_reader.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_data_writer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_datagram_queue.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_default_clock.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_default_packet_writer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_dispatcher.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_dispatcher_stats.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_error_codes.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_flow_controller.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_framer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_generic_session.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_idle_network_detector.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_mtu_discovery.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_network_blackhole_detector.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_packet_creator.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_packet_number.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_packet_reader.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_packet_writer_wrapper.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_packets.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_path_validator.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_ping_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_queue_alarm_factory.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_received_packet_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_sent_packet_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_server_id.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_session.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_socket_address_coder.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_id_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_priority.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_send_buffer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_send_buffer_base.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_sequencer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_stream_sequencer_buffer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_tag.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_time.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_time_wait_list_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_transmission_info.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_types.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_unacked_packet_map.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_utils.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_version_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_versions.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_write_blocked_list.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/deterministic_connection_id_generator.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/legacy_quic_stream_id_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/uber_quic_stream_id_manager.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/uber_received_packet_manager.cc
    
    # QUIC帧处理
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_ack_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_ack_frequency_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_blocked_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_connection_close_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_crypto_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_datagram_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_goaway_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_handshake_done_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_max_streams_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_new_connection_id_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_new_token_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_padding_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_path_challenge_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_path_response_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_ping_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_retire_connection_id_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_rst_stream_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_stop_sending_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_stream_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_streams_blocked_frame.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/frames/quic_window_update_frame.cc
    
    # QUIC加密实现
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aead_base_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aead_base_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_128_gcm_12_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_128_gcm_12_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_128_gcm_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_128_gcm_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_256_gcm_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_256_gcm_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_base_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/aes_base_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha_base_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha_base_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha20_poly1305_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha20_poly1305_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha20_poly1305_tls_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/chacha20_poly1305_tls_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/crypto_framer.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/crypto_handshake.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/crypto_handshake_message.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/crypto_utils.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/null_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/null_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_crypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_crypto_client_config.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_crypto_server_config.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_decrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_encrypter.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/quic_hkdf.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/crypto/transport_parameters.cc
    
    # QUIC握手实现
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_client_handshaker.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_client_stream.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_handshaker.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_server_stream.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_server_stream_base.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/quic_crypto_stream.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/tls_client_handshaker.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/tls_handshaker.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/tls_server_handshaker.cc
    
    # 拥塞控制算法
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/bandwidth_sampler.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/bbr_sender.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/cubic_bytes.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/general_loss_algorithm.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/hybrid_slow_start.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/pacing_sender.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/prr_sender.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/rtt_stats.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/send_algorithm_interface.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/tcp_cubic_sender_bytes.cc
    ${QUICHE_ROOT_DIR}/quiche/quic/core/congestion_control/uber_loss_algorithm.cc
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
    src/url/url_parse.cc
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

# 生成QUIC protobuf文件
set(QUICHE_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src/third-party/quiche/quiche/quic/core/proto)
set(QUICHE_PROTO_OUTPUT_DIR ${CMAKE_CURRENT_BINARY_DIR}/generated)

file(MAKE_DIRECTORY ${QUICHE_PROTO_OUTPUT_DIR})

# 生成cached_network_parameters.pb.h和.pb.cc
add_custom_command(
    OUTPUT 
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/cached_network_parameters.pb.h
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/cached_network_parameters.pb.cc
    COMMAND 
        $<TARGET_FILE:protobuf::protoc> 
        --cpp_out=${QUICHE_PROTO_OUTPUT_DIR}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src/third-party
        src/third-party/quiche/quiche/quic/core/proto/cached_network_parameters.proto
    DEPENDS 
        ${QUICHE_PROTO_DIR}/cached_network_parameters.proto
        protobuf::protoc
    COMMENT "Generating cached_network_parameters protobuf files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# 生成source_address_token.pb.h和.pb.cc
add_custom_command(
    OUTPUT 
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/source_address_token.pb.h
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/source_address_token.pb.cc
    COMMAND 
        $<TARGET_FILE:protobuf::protoc> 
        --cpp_out=${QUICHE_PROTO_OUTPUT_DIR}
        --proto_path=${CMAKE_CURRENT_SOURCE_DIR}/src/third-party
        src/third-party/quiche/quiche/quic/core/proto/source_address_token.proto
    DEPENDS 
        ${QUICHE_PROTO_DIR}/source_address_token.proto
        ${QUICHE_PROTO_DIR}/cached_network_parameters.proto
        protobuf::protoc
    COMMENT "Generating source_address_token protobuf files"
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
)

# 创建protobuf目标
add_custom_target(quiche_protos
    DEPENDS 
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/cached_network_parameters.pb.h
        ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/source_address_token.pb.h
)

# 让主目标依赖protobuf生成
add_dependencies(${PROJECT_NAME} quiche_protos)

# 添加生成的protobuf源文件到目标
target_sources(${PROJECT_NAME} PRIVATE
    ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/cached_network_parameters.pb.cc
    ${QUICHE_PROTO_OUTPUT_DIR}/quiche/quic/core/proto/source_address_token.pb.cc
)

# 添加生成的proto包含目录
target_include_directories(${PROJECT_NAME} PRIVATE 
    ${CMAKE_CURRENT_BINARY_DIR}/generated
)