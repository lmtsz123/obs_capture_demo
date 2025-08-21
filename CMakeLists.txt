cmake_minimum_required(VERSION 3.15)
project(obs_capture_demo)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找依赖包
# find_package(PkgConfig REQUIRED)  # Windows下不需要PkgConfig

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

# 包含目录
include_directories(
    ${SDL2_INCLUDE_DIR}
    ${OPENGL_INCLUDE_DIRS}
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
)

# 创建可执行文件
add_executable(${PROJECT_NAME} ${SOURCES})

# 链接库
target_link_libraries(${PROJECT_NAME}
    ${SDL2_LIBRARY}
    ${SDL2MAIN_LIBRARY}
    ${OPENGL_LIBRARIES}
    ${WINDOWS_LIBS}
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