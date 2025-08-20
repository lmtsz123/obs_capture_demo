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

# 源文件
set(SOURCES
    src/main.cpp
    src/camera_capture.cpp
    src/nv12_to_rgba_shader.cpp
    src/d3d11_renderer.cpp
    src/renderer_factory.cpp
    src/dshow_device_enum.cpp
    src/dshow_capture.cpp
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

# 复制SDL2 DLL到输出目录（Windows平台）
if(WIN32)
    add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${SDL2_LIB_DIR}/SDL2.dll"
        $<TARGET_FILE_DIR:${PROJECT_NAME}>)
endif()