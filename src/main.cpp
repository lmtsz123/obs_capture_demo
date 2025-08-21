#include <SDL.h>
#include <SDL_opengl.h>
#include <SDL_syswm.h>
#include <iostream>
#include <vector>
#include <algorithm>
#include <cctype>
#include <cstring>
#include <mutex>

#include "camera_capture.h"
#include "dshow_capture.h"
#include "renderer_interface.h"
#include "nv12_to_rgba_shader.h"  // 为了使用InitializeOpenGLExtensions函数
#include "d3d11_renderer.h"
#include "vulkan_renderer.h"

class Application {
private:
    SDL_Window* m_window;
    SDL_GLContext m_glContext;
    CameraCapture m_capture;
    DirectShowCapture m_dshowCapture;
    IRenderer* m_renderer;
    bool m_running;
    
    // 帧数据缓存
    std::vector<uint8_t> m_latestFrame;
    std::vector<uint8_t> m_renderFrame;  // 用于渲染的帧缓存
    int m_frameWidth;
    int m_frameHeight;
    bool m_hasNewFrame;
    bool m_frameReady;  // 标记是否有准备好的帧用于渲染
    std::mutex m_frameMutex;
    
    static const int WINDOW_WIDTH = 1080;
    static const int WINDOW_HEIGHT = 1920;

public:
    Application() : m_window(nullptr), m_glContext(nullptr), m_renderer(nullptr), m_running(false),
                    m_frameWidth(0), m_frameHeight(0), m_hasNewFrame(false), m_frameReady(false) {}
    
    ~Application() {
        Cleanup();
    }
    
    bool Initialize() {
        // 初始化SDL
        if (SDL_Init(SDL_INIT_VIDEO) < 0) {
            std::cerr << "SDL_Init failed: " << SDL_GetError() << std::endl;
            return false;
        }
        
        // 选择渲染器类型（可以通过命令行参数或配置文件选择）
        RendererType rendererType = RendererType::Vulkan;  // 默认使用Direct3D11
        
        // 根据渲染器类型创建窗口
        if (rendererType == RendererType::OpenGL) {
            // 设置OpenGL属性
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
            SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
            SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
            
            // 创建OpenGL窗口
            m_window = SDL_CreateWindow("OBS Virtual Camera Capture",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL | SDL_WINDOW_SHOWN);
            
            if (!m_window) {
                std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
                return false;
            }
            
            // 创建OpenGL上下文
            m_glContext = SDL_GL_CreateContext(m_window);
            if (!m_glContext) {
                std::cerr << "SDL_GL_CreateContext failed: " << SDL_GetError() << std::endl;
                return false;
            }
            
            // 启用垂直同步
            SDL_GL_SetSwapInterval(1);
            
            // 初始化OpenGL
            glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
            glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
        } else {
            // 创建D3D11兼容的窗口（不使用OpenGL标志）
            m_window = SDL_CreateWindow("OBS Virtual Camera Capture",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_SHOWN);
            
            if (!m_window) {
                std::cerr << "SDL_CreateWindow failed: " << SDL_GetError() << std::endl;
                return false;
            }
            
            m_glContext = nullptr;  // D3D11不需要OpenGL上下文
        }
        
        // 如果使用OpenGL，初始化扩展函数
        if (rendererType == RendererType::OpenGL) {
            if (!InitializeOpenGLExtensions()) {
                std::cerr << "Failed to initialize OpenGL extensions" << std::endl;
                return false;
            }
        }
        
        // 创建渲染器
        m_renderer = RendererFactory::CreateRenderer(rendererType);
        if (!m_renderer) {
            std::cerr << "Failed to create renderer" << std::endl;
            return false;
        }
        
        // 初始化渲染器
        bool rendererInitialized = false;
        if (rendererType == RendererType::Direct3D11) {
            // D3D11渲染器需要窗口句柄
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            if (SDL_GetWindowWMInfo(m_window, &wmInfo)) {
                HWND hwnd = wmInfo.info.win.window;
                D3D11Renderer* d3d11Renderer = static_cast<D3D11Renderer*>(m_renderer);
                rendererInitialized = d3d11Renderer->Initialize(hwnd);
            } else {
                std::cerr << "Failed to get window handle for D3D11" << std::endl;
            }
        } else if (rendererType == RendererType::Vulkan) {
            // Vulkan渲染器需要窗口句柄
            SDL_SysWMinfo wmInfo;
            SDL_VERSION(&wmInfo.version);
            if (SDL_GetWindowWMInfo(m_window, &wmInfo)) {
                HWND hwnd = wmInfo.info.win.window;
                VulkanRenderer* vulkanRenderer = static_cast<VulkanRenderer*>(m_renderer);
                rendererInitialized = vulkanRenderer->Initialize(hwnd);
            } else {
                std::cerr << "Failed to get window handle for Vulkan" << std::endl;
            }
        } else {
            // OpenGL渲染器使用默认初始化
            rendererInitialized = m_renderer->Initialize();
        }
        
        if (!rendererInitialized) {
            std::cerr << "Failed to initialize renderer" << std::endl;
            return false;
        }
        
        std::string rendererName;
        switch (rendererType) {
            case RendererType::OpenGL: rendererName = "OpenGL"; break;
            case RendererType::Direct3D11: rendererName = "Direct3D11"; break;
            case RendererType::Vulkan: rendererName = "Vulkan"; break;
            default: rendererName = "Unknown"; break;
        }
        std::cout << "Using renderer: " << rendererName << std::endl;
        
        // 初始化摄像头捕获
        if (!m_capture.Initialize()) {
            std::cerr << "Failed to initialize camera capture" << std::endl;
            return false;
        }
        
        // 初始化DirectShow捕获
        if (!m_dshowCapture.Initialize()) {
            std::cerr << "Failed to initialize DirectShow capture" << std::endl;
            return false;
        }
        
        return true;
    }
    
    void Run() {
        // 使用DirectShow枚举设备
        auto dshowDevices = m_capture.GetDirectShowDevices();
        std::cout << "Available devices (DirectShow):" << std::endl;
        for (size_t i = 0; i < dshowDevices.size(); i++) {
            std::cout << i << ": " << dshowDevices[i].friendlyName;
            if (dshowDevices[i].isOBSVirtualCamera) {
                std::cout << " [OBS Virtual Camera]";
            }
            std::cout << std::endl;
        }
        
        // 让用户选择摄像头设备
        std::string selectedDevice;
        
        if (dshowDevices.empty()) {
            std::cerr << "No camera devices found!" << std::endl;
            return;
        }
        
        // 检查是否找到OBS Virtual Camera
        DirectShowDevice* obsDevice = m_capture.FindOBSVirtualCamera();
        if (obsDevice) {
            std::cout << "\nFound OBS Virtual Camera: " << obsDevice->friendlyName << std::endl;
            std::cout << "Would you like to use it? (y/n, default: y): ";
            std::string useOBS;
            std::getline(std::cin, useOBS);
            
            if (useOBS.empty() || useOBS[0] == 'y' || useOBS[0] == 'Y') {
                selectedDevice = obsDevice->friendlyName;
                std::cout << "Using OBS Virtual Camera." << std::endl;
            }
        }
        
        // 如果没有选择OBS或者没找到OBS，让用户手动选择
        if (selectedDevice.empty()) {
            std::cout << "\nPlease select a camera device:" << std::endl;
            for (size_t i = 0; i < dshowDevices.size(); i++) {
                std::cout << i << ": " << dshowDevices[i].friendlyName;
                if (dshowDevices[i].isOBSVirtualCamera) {
                    std::cout << " [OBS Virtual Camera]";
                }
                std::cout << std::endl;
            }
            
            std::cout << "Enter device number (0-" << (dshowDevices.size()-1) << "): ";
            std::string input;
            std::getline(std::cin, input);
            
            size_t deviceIndex = 0;
            if (!input.empty()) {
                try {
                    deviceIndex = std::stoul(input);
                    if (deviceIndex >= dshowDevices.size()) {
                        std::cout << "Invalid device number, using device 0." << std::endl;
                        deviceIndex = 0;
                    }
                } catch (const std::exception&) {
                    std::cout << "Invalid input, using device 0." << std::endl;
                    deviceIndex = 0;
                }
            }
            
            selectedDevice = dshowDevices[deviceIndex].friendlyName;
        }
        
        std::cout << "Selected device: " << selectedDevice << std::endl;
        std::cout << "Starting capture..." << std::endl;
        
        bool captureStarted = false;
        
        // 如果选择的是OBS Virtual Camera，优先使用DirectShow
        if (selectedDevice.find("OBS Virtual Camera") != std::string::npos || 
            selectedDevice.find("OBS-Camera") != std::string::npos) {
            std::cout << "Using DirectShow for OBS Virtual Camera..." << std::endl;
            captureStarted = m_dshowCapture.StartCapture(selectedDevice,
                [this](const DirectShowFrameData& frame) {
                    this->OnDirectShowFrameReceived(frame);
                });
        }
        
        // 如果DirectShow失败或不是OBS设备，尝试Media Foundation
        if (!captureStarted) {
            std::cout << "Using Media Foundation..." << std::endl;
            captureStarted = m_capture.StartCapture(selectedDevice,
                [this](const CameraCapture::FrameData& frame) {
                    this->OnFrameReceived(frame);
                });
        }
            
        if (!captureStarted) {
            std::cerr << "Failed to start capture for device: " << selectedDevice << std::endl;
            std::cerr << "This might be because:" << std::endl;
            std::cerr << "1. The device is already in use by another application" << std::endl;
            std::cerr << "2. The device doesn't support the required format" << std::endl;
            std::cerr << "3. Access permissions issue" << std::endl;
            return;
        }
        
        std::cout << "Capture started successfully!" << std::endl;
        std::cout << "Press ESC to exit, or close the window." << std::endl;
        std::cout << "Starting main loop..." << std::endl;
        
        m_running = true;
        SDL_Event event;
        
        int loopCount = 0;
        while (m_running) {
            loopCount++;
            if (loopCount <= 10 || loopCount % 100 == 0) {
                std::cout << "Main loop iteration: " << loopCount << std::endl;
            }
            
            // 处理事件
            while (SDL_PollEvent(&event)) {
                if (event.type == SDL_QUIT) {
                    std::cout << "SDL_QUIT event received" << std::endl;
                    m_running = false;
                }
                else if (event.type == SDL_KEYDOWN) {
                    if (event.key.keysym.sym == SDLK_ESCAPE) {
                        std::cout << "ESC key pressed" << std::endl;
                        m_running = false;
                    }
                }
            }
            
            // 渲染
            Render();
            
            SDL_Delay(33); // ~30fps，降低帧率减少闪烁
        }
        
        std::cout << "Main loop exited after " << loopCount << " iterations" << std::endl;
        
        m_capture.StopCapture();
        m_dshowCapture.StopCapture();
    }
    
private:
    void OnFrameReceived(const CameraCapture::FrameData& frame) {
        // 这个回调在捕获线程中调用
        // 使用互斥锁保护帧数据
        if (frame.data && frame.size > 0) {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            m_latestFrame.resize(frame.size);
            memcpy(m_latestFrame.data(), frame.data, frame.size);
            m_frameWidth = frame.width;
            m_frameHeight = frame.height;
            m_hasNewFrame = true;
            
            // 只在前几帧打印详细信息，避免刷屏
            static int frameCount = 0;
            if (frameCount < 5) {
                std::cout << "Frame " << frameCount << ": " << frame.width << "x" << frame.height 
                          << " size: " << frame.size << " bytes" << std::endl;
                frameCount++;
            }
        }
    }
    
    void OnDirectShowFrameReceived(const DirectShowFrameData& frame) {
        // DirectShow帧接收回调
        if (!frame.data.empty()) {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            
            // 根据格式处理帧数据
            if (frame.format == MEDIASUBTYPE_NV12) {
                // NV12格式直接使用
                m_latestFrame = frame.data;
            } else if (frame.format == MEDIASUBTYPE_RGB24) {
                // RGB24格式需要转换为NV12
                ConvertRGB24ToNV12(frame.data.data(), frame.width, frame.height);
                std::cout << "Converted RGB24 to NV12" << std::endl;
            } else {
                // 其他格式尝试直接使用
                m_latestFrame = frame.data;
                std::cout << "Using unknown format directly, size: " << frame.data.size() << std::endl;
            }
            
            m_frameWidth = frame.width;
            m_frameHeight = frame.height;
            m_hasNewFrame = true;
            
            static int frameCount = 0;
            frameCount++;
            if (frameCount <= 3) {  // 只显示前3帧的信息
                std::cout << "DirectShow Frame " << frameCount << ": " 
                         << frame.width << "x" << frame.height 
                         << ", size: " << frame.data.size() << " bytes" << std::endl;
            }
        }
    }
    
    void ConvertRGB24ToNV12(const uint8_t* rgb24Data, int width, int height) {
        // RGB24到NV12转换
        size_t nv12Size = width * height * 3 / 2;
        if (m_latestFrame.size() != nv12Size) {
            m_latestFrame.resize(nv12Size);
        }
        
        uint8_t* yPlane = m_latestFrame.data();
        uint8_t* uvPlane = yPlane + width * height;
        
        // RGB24到YUV转换（BGR顺序）
        for (int y = 0; y < height; y++) {
            for (int x = 0; x < width; x++) {
                int rgbIndex = (y * width + x) * 3;
                int yIndex = y * width + x;
                
                uint8_t b = rgb24Data[rgbIndex];
                uint8_t g = rgb24Data[rgbIndex + 1];
                uint8_t r = rgb24Data[rgbIndex + 2];
                
                // Y分量
                yPlane[yIndex] = (uint8_t)(0.299 * r + 0.587 * g + 0.114 * b);
                
                // UV分量（每2x2像素一个）
                if (x % 2 == 0 && y % 2 == 0) {
                    int uvIndex = (y / 2) * width + (x / 2) * 2;
                    if (uvIndex + 1 < width * height / 2) {
                        uvPlane[uvIndex] = (uint8_t)(128 - 0.168736 * r - 0.331264 * g + 0.5 * b);     // U
                        uvPlane[uvIndex + 1] = (uint8_t)(128 + 0.5 * r - 0.418688 * g - 0.081312 * b); // V
                    }
                }
            }
        }
    }
    
    void Render() {
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 检查是否有新帧需要更新渲染缓存
        {
            std::lock_guard<std::mutex> lock(m_frameMutex);
            if (m_hasNewFrame && !m_latestFrame.empty()) {
                m_renderFrame = m_latestFrame;  // 更新渲染缓存
                m_frameReady = true;
                m_hasNewFrame = false;  // 标记帧已处理
            }
        }
        
        // 使用渲染缓存进行渲染（不需要锁）
        if (m_frameReady && m_renderer && !m_renderFrame.empty()) {
            static int renderCount = 0;
            renderCount++;
            if (renderCount <= 3) {
                std::cout << "Rendering frame " << renderCount << ": " << m_frameWidth << "x" << m_frameHeight 
                         << ", data size: " << m_renderFrame.size() << std::endl;
            }
            // 使用渲染器渲染帧数据到屏幕
            m_renderer->RenderToScreen(m_renderFrame.data(), m_frameWidth, m_frameHeight);
        } else {
            // 没有帧数据时显示默认背景
            glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
            glClear(GL_COLOR_BUFFER_BIT);
        }
        
        SDL_GL_SwapWindow(m_window);
    }
    
    void Cleanup() {
        if (m_renderer) {
            RendererFactory::DestroyRenderer(m_renderer);
            m_renderer = nullptr;
        }
        m_capture.Cleanup();
        
        if (m_glContext) {
            SDL_GL_DeleteContext(m_glContext);
            m_glContext = nullptr;
        }
        
        if (m_window) {
            SDL_DestroyWindow(m_window);
            m_window = nullptr;
        }
        
        SDL_Quit();
    }
};

int main(int argc, char* argv[]) {
    Application app;
    
    if (!app.Initialize()) {
        std::cerr << "Failed to initialize application" << std::endl;
        return -1;
    }
    
    app.Run();
    
    return 0;
}