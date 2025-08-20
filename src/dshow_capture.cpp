#include "dshow_capture.h"
#include "dshow_device_enum.h"
#include <iostream>
#include <iomanip>

// GUID定义
DEFINE_GUID(CLSID_SampleGrabber, 0xc1f400a0, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);
DEFINE_GUID(IID_ISampleGrabber, 0x6b652fff, 0x11fe, 0x4fce, 0x92, 0xad, 0x02, 0x66, 0xb5, 0xd7, 0xc7, 0x8f);
DEFINE_GUID(CLSID_NullRenderer, 0xc1f400a4, 0x3f08, 0x11d3, 0x9f, 0x0b, 0x00, 0x60, 0x08, 0x03, 0x9e, 0x37);

DirectShowCapture::DirectShowCapture() 
    : m_initialized(false), m_capturing(false), m_grabberCallback(nullptr) {
}

DirectShowCapture::~DirectShowCapture() {
    Cleanup();
}

bool DirectShowCapture::Initialize() {
    if (m_initialized) return true;

    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::cerr << "Failed to initialize COM. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }

    m_initialized = true;
    return true;
}

void DirectShowCapture::Cleanup() {
    StopCapture();
    
    if (m_grabberCallback) {
        delete m_grabberCallback;
        m_grabberCallback = nullptr;
    }
    
    m_nullRenderer.Release();
    m_sampleGrabber.Release();
    m_sourceFilter.Release();
    m_mediaControl.Release();
    m_captureBuilder.Release();
    m_graphBuilder.Release();
    
    if (m_initialized) {
        CoUninitialize();
        m_initialized = false;
    }
}

bool DirectShowCapture::StartCapture(const std::string& deviceName, FrameCallback callback) {
    if (!m_initialized || m_capturing) return false;
    
    std::cout << "Starting DirectShow capture for device: " << deviceName << std::endl;
    
    m_frameCallback = callback;
    
    // 创建捕获图
    if (!CreateCaptureGraph()) {
        std::cerr << "Failed to create capture graph" << std::endl;
        return false;
    }
    
    // 查找设备
    if (!FindCaptureDevice(deviceName, &m_sourceFilter)) {
        std::cerr << "Failed to find capture device: " << deviceName << std::endl;
        return false;
    }
    
    // 配置图
    if (!ConfigureGraph()) {
        std::cerr << "Failed to configure capture graph" << std::endl;
        return false;
    }
    
    // 先设置捕获标志，然后再启动图
    m_capturing = true;
    
    // 开始捕获
    HRESULT hr = m_mediaControl->Run();
    if (FAILED(hr)) {
        std::cerr << "Failed to start capture. HRESULT: " << std::hex << hr << std::endl;
        m_capturing = false;
        return false;
    }
    
    // 等待一下让图稳定
    Sleep(100);
    
    // 检查图的状态
    OAFilterState state;
    hr = m_mediaControl->GetState(1000, &state);
    if (SUCCEEDED(hr)) {
        std::cout << "Graph state: " << (state == State_Running ? "Running" : 
                                        state == State_Paused ? "Paused" : "Stopped") << std::endl;
    }
    std::cout << "DirectShow capture started successfully" << std::endl;
    
    // 启动一个监控线程来检查帧率
    std::thread([this]() {
        int lastFrameCount = 0;
        for (int i = 0; i < 10 && m_capturing; i++) {
            Sleep(1000); // 等待1秒
            // 这里可以添加帧计数检查
            std::cout << "Monitoring: " << (i + 1) << " seconds elapsed" << std::endl;
        }
    }).detach();
    
    return true;
}

void DirectShowCapture::StopCapture() {
    if (!m_capturing) return;
    
    m_capturing = false;
    
    if (m_mediaControl) {
        m_mediaControl->Stop();
    }
    
    std::cout << "DirectShow capture stopped" << std::endl;
}

bool DirectShowCapture::CreateCaptureGraph() {
    HRESULT hr;
    
    // 创建Filter Graph Manager
    hr = CoCreateInstance(CLSID_FilterGraph, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&m_graphBuilder));
    if (FAILED(hr)) {
        std::cerr << "Failed to create FilterGraph. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 创建Capture Graph Builder
    hr = CoCreateInstance(CLSID_CaptureGraphBuilder2, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&m_captureBuilder));
    if (FAILED(hr)) {
        std::cerr << "Failed to create CaptureGraphBuilder2. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 设置Filter Graph
    hr = m_captureBuilder->SetFiltergraph(m_graphBuilder);
    if (FAILED(hr)) {
        std::cerr << "Failed to set filter graph. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 获取Media Control接口
    hr = m_graphBuilder->QueryInterface(IID_PPV_ARGS(&m_mediaControl));
    if (FAILED(hr)) {
        std::cerr << "Failed to get MediaControl interface. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    return true;
}

bool DirectShowCapture::FindCaptureDevice(const std::string& deviceName, IBaseFilter** sourceFilter) {
    DirectShowDeviceEnumerator enumerator;
    if (!enumerator.Initialize()) {
        std::cerr << "Failed to initialize DirectShow enumerator" << std::endl;
        return false;
    }
    
    auto devices = enumerator.EnumerateVideoDevices();
    
    for (const auto& device : devices) {
        if (device.friendlyName == deviceName) {
            std::cout << "Found device: " << device.friendlyName << std::endl;
            
            // 创建设备过滤器
            HRESULT hr = CoCreateInstance(device.clsid, nullptr, CLSCTX_INPROC_SERVER,
                                        IID_PPV_ARGS(sourceFilter));
            if (SUCCEEDED(hr)) {
                std::cout << "Successfully created device filter" << std::endl;
                return true;
            } else {
                std::cerr << "Failed to create device filter. HRESULT: " << std::hex << hr << std::endl;
            }
        }
    }
    
    std::cerr << "Device not found: " << deviceName << std::endl;
    return false;
}

bool DirectShowCapture::ConfigureGraph() {
    HRESULT hr;
    
    // 添加源过滤器到图中
    hr = m_graphBuilder->AddFilter(m_sourceFilter, L"Video Capture");
    if (FAILED(hr)) {
        std::cerr << "Failed to add source filter. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 设置Sample Grabber
    if (!SetupSampleGrabber()) {
        std::cerr << "Failed to setup sample grabber" << std::endl;
        return false;
    }
    
    // 创建Null Renderer
    hr = CoCreateInstance(CLSID_NullRenderer, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&m_nullRenderer));
    if (FAILED(hr)) {
        std::cerr << "Failed to create null renderer. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 添加Null Renderer到图中
    hr = m_graphBuilder->AddFilter(m_nullRenderer, L"Null Renderer");
    if (FAILED(hr)) {
        std::cerr << "Failed to add null renderer. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 使用OBS Virtual Camera的特殊连接策略
    std::cout << "Attempting OBS Virtual Camera connection strategy..." << std::endl;
    
    // 尝试不同的PIN类别，按优先级顺序
    hr = m_captureBuilder->RenderStream(&PIN_CATEGORY_PREVIEW, &MEDIATYPE_Video,
                                       m_sourceFilter, m_sampleGrabber, m_nullRenderer);
    
    if (FAILED(hr)) {
        std::cout << "PREVIEW connection failed, trying CAPTURE..." << std::endl;
        hr = m_captureBuilder->RenderStream(&PIN_CATEGORY_CAPTURE, &MEDIATYPE_Video,
                                           m_sourceFilter, m_sampleGrabber, m_nullRenderer);
    }
    
    if (FAILED(hr)) {
        std::cout << "CAPTURE connection failed, trying without category..." << std::endl;
        hr = m_captureBuilder->RenderStream(NULL, &MEDIATYPE_Video,
                                           m_sourceFilter, m_sampleGrabber, m_nullRenderer);
    }
    
    if (FAILED(hr)) {
        std::cerr << "Failed to render stream with all methods. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    std::cout << "Stream connection successful!" << std::endl;
    
    std::cout << "Capture graph configured successfully" << std::endl;
    
    // 检查连接后的媒体类型
    CComPtr<ISampleGrabber> grabber;
    hr = m_sampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&grabber);
    if (SUCCEEDED(hr)) {
        AM_MEDIA_TYPE mt;
        hr = grabber->GetConnectedMediaType(&mt);
        if (SUCCEEDED(hr)) {
            std::cout << "Connected media type: ";
            if (mt.subtype == MEDIASUBTYPE_RGB24) {
                std::cout << "RGB24";
            } else if (mt.subtype == MEDIASUBTYPE_NV12) {
                std::cout << "NV12";
            } else if (mt.subtype == MEDIASUBTYPE_YUY2) {
                std::cout << "YUY2";
            } else {
                std::cout << "Unknown format";
            }
            
            if (mt.formattype == FORMAT_VideoInfo) {
                VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;
                std::cout << ", Size: " << vih->bmiHeader.biWidth << "x" << abs(vih->bmiHeader.biHeight);
            }
            std::cout << std::endl;
            
            // 清理媒体类型
            if (mt.cbFormat != 0) {
                CoTaskMemFree((PVOID)mt.pbFormat);
            }
            if (mt.pUnk != nullptr) {
                mt.pUnk->Release();
            }
        }
    }
    
    return true;
}

bool DirectShowCapture::SetupSampleGrabber() {
    HRESULT hr;
    
    // 创建Sample Grabber
    hr = CoCreateInstance(CLSID_SampleGrabber, nullptr, CLSCTX_INPROC_SERVER,
                         IID_PPV_ARGS(&m_sampleGrabber));
    if (FAILED(hr)) {
        std::cerr << "Failed to create sample grabber. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 添加到图中
    hr = m_graphBuilder->AddFilter(m_sampleGrabber, L"Sample Grabber");
    if (FAILED(hr)) {
        std::cerr << "Failed to add sample grabber. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 获取ISampleGrabber接口
    CComPtr<ISampleGrabber> grabber;
    hr = m_sampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&grabber);
    if (FAILED(hr)) {
        std::cerr << "Failed to get ISampleGrabber interface. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 先不设置媒体类型，让DirectShow自动协商
    std::cout << "Setting up media type (auto-negotiation)..." << std::endl;
    
    // 不设置特定媒体类型，让DirectShow自动选择最佳格式
    // 这样可以确保与OBS Virtual Camera的兼容性
    
    // 创建回调对象
    m_grabberCallback = new SampleGrabberCallback(this);
    
    // 按照工作代码的顺序：先设置参数，再设置回调
    
    // 1. 设置连续获取（不是单次）
    hr = grabber->SetOneShot(FALSE);
    if (FAILED(hr)) {
        std::cerr << "Failed to set one shot. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 2. 启用缓冲采样（关键！）
    hr = grabber->SetBufferSamples(TRUE);
    if (FAILED(hr)) {
        std::cerr << "Failed to set buffer samples. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 3. 最后设置回调 - 优先使用BufferCB模式（更稳定）
    hr = grabber->SetCallback(m_grabberCallback, 1); // 1 = BufferCB mode
    if (FAILED(hr)) {
        std::cout << "BufferCB mode failed, trying SampleCB mode..." << std::endl;
        hr = grabber->SetCallback(m_grabberCallback, 0); // 0 = SampleCB mode
        if (FAILED(hr)) {
            std::cerr << "Failed to set any callback mode. HRESULT: " << std::hex << hr << std::endl;
            return false;
        } else {
            std::cout << "SampleCB mode set successfully" << std::endl;
        }
    } else {
        std::cout << "BufferCB mode set successfully" << std::endl;
    }
    
    std::cout << "Sample grabber setup successfully" << std::endl;
    return true;
}

void DirectShowCapture::OnFrameReceived(const uint8_t* buffer, long bufferSize, double sampleTime) {
    if (!m_capturing || !m_frameCallback) {
        std::cout << "OnFrameReceived: capturing=" << m_capturing << ", callback=" << (m_frameCallback ? "yes" : "no") << std::endl;
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_frameMutex);
    
    // 获取视频格式信息
    CComPtr<ISampleGrabber> grabber;
    HRESULT hr = m_sampleGrabber->QueryInterface(IID_ISampleGrabber, (void**)&grabber);
    if (FAILED(hr)) {
        std::cout << "OnFrameReceived: Failed to get ISampleGrabber interface" << std::endl;
        return;
    }
    
    AM_MEDIA_TYPE mt;
    hr = grabber->GetConnectedMediaType(&mt);
    if (FAILED(hr)) {
        std::cout << "OnFrameReceived: Failed to get connected media type" << std::endl;
        return;
    }
    
    if (mt.formattype == FORMAT_VideoInfo) {
        VIDEOINFOHEADER* vih = (VIDEOINFOHEADER*)mt.pbFormat;
        
        m_currentFrame.width = vih->bmiHeader.biWidth;
        m_currentFrame.height = abs(vih->bmiHeader.biHeight);
        m_currentFrame.format = mt.subtype;
        m_currentFrame.timestamp = static_cast<uint64_t>(sampleTime * 10000000); // 转换为100ns单位
        
        // 复制帧数据
        m_currentFrame.data.assign(buffer, buffer + bufferSize);
        
        static int callbackCount = 0;
        callbackCount++;
        if (callbackCount <= 3) {
            std::cout << "OnFrameReceived: Calling callback with " << bufferSize << " bytes, " 
                      << m_currentFrame.width << "x" << m_currentFrame.height << std::endl;
        }
        
        // 调用回调
        m_frameCallback(m_currentFrame);
    } else {
        std::cout << "OnFrameReceived: Unsupported format type" << std::endl;
    }
    
    if (mt.cbFormat != 0) {
        CoTaskMemFree((PVOID)mt.pbFormat);
    }
    if (mt.pUnk != nullptr) {
        mt.pUnk->Release();
    }
}

// SampleGrabberCallback实现
SampleGrabberCallback::SampleGrabberCallback(DirectShowCapture* parent)
    : m_parent(parent), m_refCount(1) {
}

SampleGrabberCallback::~SampleGrabberCallback() {
}

STDMETHODIMP SampleGrabberCallback::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_ISampleGrabberCB) {
        *ppv = static_cast<ISampleGrabberCB*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) SampleGrabberCallback::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) SampleGrabberCallback::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

STDMETHODIMP SampleGrabberCallback::SampleCB(double sampleTime, IMediaSample* sample) {
    static int sampleCallCount = 0;
    sampleCallCount++;
    
    if (sampleCallCount <= 10) {
        std::cout << "SampleCB called: " << sampleCallCount << ", Time=" << sampleTime << std::endl;
    } else if (sampleCallCount % 30 == 0) {
        std::cout << "SampleCB " << sampleCallCount << " (continuing...)" << std::endl;
    }
    
    if (!sample) {
        std::cout << "SampleCB: sample is NULL!" << std::endl;
        return E_POINTER;
    }
    
    // 获取样本数据
    BYTE* pBuffer = NULL;
    HRESULT hr = sample->GetPointer(&pBuffer);
    if (FAILED(hr) || !pBuffer) {
        if (sampleCallCount <= 3) {
            std::cout << "SampleCB: Failed to get sample pointer, hr=0x" << std::hex << hr << std::dec << std::endl;
        }
        return hr;
    }
    
    // 获取样本大小
    long lDataLen = sample->GetActualDataLength();
    if (sampleCallCount <= 3) {
        std::cout << "SampleCB: Got sample with " << lDataLen << " bytes" << std::endl;
    }
    
    // 调用BufferCB来处理数据（重用现有逻辑）
    return BufferCB(sampleTime, pBuffer, lDataLen);
}

STDMETHODIMP SampleGrabberCallback::BufferCB(double sampleTime, BYTE* buffer, long bufferLen) {
    static int callbackCount = 0;
    callbackCount++;
    
    if (callbackCount <= 3) {
        std::cout << "BufferCB called: " << callbackCount << ", bufferLen: " << bufferLen << std::endl;
    }
    
    if (m_parent && buffer && bufferLen > 0) {
        try {
            m_parent->OnFrameReceived(buffer, bufferLen, sampleTime);
        } catch (...) {
            std::cout << "Exception in OnFrameReceived" << std::endl;
        }
    }
    return S_OK;
}
