#include "wmf_capture.h"
#include <iostream>
#include <mfcaptureengine.h>
#include <devicetopology.h>

WMFCapture::WMFCapture() 
    : m_initialized(false), m_capturing(false), m_readerCallback(nullptr), m_resetToken(0) {
}

WMFCapture::~WMFCapture() {
    Cleanup();
}

bool WMFCapture::Initialize() {
    if (m_initialized) return true;

    // 初始化COM
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to initialize COM. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }

    // 初始化Media Foundation
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to initialize Media Foundation. HRESULT: " << std::hex << hr << std::endl;
        CoUninitialize();
        return false;
    }

    // 初始化D3D11
    if (!InitializeD3D11()) {
        std::wcerr << L"Failed to initialize D3D11" << std::endl;
        MFShutdown();
        CoUninitialize();
        return false;
    }

    m_initialized = true;
    std::wcout << L"WMF Capture initialized successfully" << std::endl;
    return true;
}

void WMFCapture::Cleanup() {
    StopCapture();
    
    if (m_readerCallback) {
        m_readerCallback->Release();
        m_readerCallback = nullptr;
    }
    
    if (m_dxgiManager) {
        m_dxgiManager->ResetDevice(m_d3d11Device.Get(), m_resetToken);
        m_dxgiManager.Reset();
    }
    
    m_sourceReader.Reset();
    m_d3d11Context.Reset();
    m_d3d11Device.Reset();
    
    if (m_initialized) {
        MFShutdown();
        CoUninitialize();
        m_initialized = false;
    }
}

bool WMFCapture::InitializeD3D11() {
    HRESULT hr;
    
    // 创建D3D11设备
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    
    UINT createDeviceFlags = D3D11_CREATE_DEVICE_VIDEO_SUPPORT;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif

    D3D_FEATURE_LEVEL featureLevel;
    hr = D3D11CreateDevice(
        nullptr,                    // 使用默认适配器
        D3D_DRIVER_TYPE_HARDWARE,   // 硬件加速
        nullptr,                    // 软件光栅化器
        createDeviceFlags,          // 创建标志
        featureLevels,              // 特性级别数组
        ARRAYSIZE(featureLevels),   // 特性级别数量
        D3D11_SDK_VERSION,          // SDK版本
        &m_d3d11Device,             // 输出设备
        &featureLevel,              // 输出特性级别
        &m_d3d11Context             // 输出设备上下文
    );
    
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create D3D11 device. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    std::wcout << L"D3D11 device created with feature level: " << std::hex << featureLevel << std::endl;
    
    // 创建DXGI设备管理器
    hr = MFCreateDXGIDeviceManager(&m_resetToken, &m_dxgiManager);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create DXGI device manager. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 将D3D11设备关联到DXGI管理器
    hr = m_dxgiManager->ResetDevice(m_d3d11Device.Get(), m_resetToken);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to reset DXGI device. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    std::wcout << L"DXGI device manager initialized successfully" << std::endl;
    return true;
}

std::vector<WMFDevice> WMFCapture::EnumerateDevices() {
    std::vector<WMFDevice> devices;
    
    if (!m_initialized) {
        std::wcerr << L"WMF not initialized" << std::endl;
        return devices;
    }
    
    ComPtr<IMFAttributes> attributes;
    HRESULT hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create attributes. HRESULT: " << std::hex << hr << std::endl;
        return devices;
    }
    
    // 设置设备类型为视频捕获
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
                            MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set device type. HRESULT: " << std::hex << hr << std::endl;
        return devices;
    }
    
    IMFActivate** ppDevices = nullptr;
    UINT32 count = 0;
    
    hr = MFEnumDeviceSources(attributes.Get(), &ppDevices, &count);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to enumerate devices. HRESULT: " << std::hex << hr << std::endl;
        return devices;
    }
    
    std::wcout << L"Found " << count << L" WMF video devices:" << std::endl;
    
    if (count == 0) {
        std::wcout << L"No video capture devices found" << std::endl;
        CoTaskMemFree(ppDevices);
        return devices;
    }
    
    for (UINT32 i = 0; i < count; i++) {
        WMFDevice device;
        
        // 获取友好名称
        WCHAR* friendlyName = nullptr;
        UINT32 nameLength = 0;
        hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME, 
                                            &friendlyName, &nameLength);
        if (SUCCEEDED(hr)) {
            device.friendlyName = friendlyName;
            CoTaskMemFree(friendlyName);
        }
        
        // 获取符号链接
        WCHAR* symbolicLink = nullptr;
        UINT32 linkLength = 0;
        hr = ppDevices[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK,
                                            &symbolicLink, &linkLength);
        if (SUCCEEDED(hr)) {
            device.symbolicLink = symbolicLink;
            CoTaskMemFree(symbolicLink);
        }
        
        // 检查硬件加速支持（简单检查）
        device.isHardwareAccelerated = (device.friendlyName.find(L"Intel") != std::wstring::npos ||
                                       device.friendlyName.find(L"NVIDIA") != std::wstring::npos ||
                                       device.friendlyName.find(L"AMD") != std::wstring::npos);
        
        devices.push_back(device);
        
        std::wcout << L"  " << i << L": " << device.friendlyName;
        if (device.isHardwareAccelerated) {
            std::wcout << L" [Hardware Accelerated]";
        }
        std::wcout << std::endl;
        
        ppDevices[i]->Release();
    }
    
    CoTaskMemFree(ppDevices);
    return devices;
}

bool WMFCapture::StartCapture(const std::wstring& deviceSymbolicLink, FrameCallback callback) {
    if (!m_initialized || m_capturing) return false;
    
    std::wcout << L"Starting WMF capture for device: " << deviceSymbolicLink << std::endl;
    
    m_frameCallback = callback;
    
    // 创建源读取器
    if (!CreateSourceReader(deviceSymbolicLink)) {
        std::wcerr << L"Failed to create source reader" << std::endl;
        return false;
    }
    
    // 配置源读取器
    if (!ConfigureSourceReader()) {
        std::wcerr << L"Failed to configure source reader" << std::endl;
        return false;
    }
    
    m_capturing = true;
    
    // 开始异步读取
    HRESULT hr = m_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 0, nullptr, nullptr, nullptr, nullptr);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to start reading samples. HRESULT: " << std::hex << hr << std::endl;
        m_capturing = false;
        return false;
    }
    
    std::wcout << L"WMF capture started successfully" << std::endl;
    return true;
}

void WMFCapture::StopCapture() {
    if (!m_capturing) return;
    
    m_capturing = false;
    
    if (m_sourceReader) {
        m_sourceReader->Flush(MF_SOURCE_READER_FIRST_VIDEO_STREAM);
    }
    
    std::wcout << L"WMF capture stopped" << std::endl;
}

bool WMFCapture::CreateSourceReader(const std::wstring& deviceSymbolicLink) {
    HRESULT hr;
    
    // 创建设备源属性
    ComPtr<IMFAttributes> sourceAttributes;
    hr = MFCreateAttributes(&sourceAttributes, 2);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create source attributes. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    hr = sourceAttributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE, 
                                  MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set source type. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    hr = sourceAttributes->SetString(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_SYMBOLIC_LINK, 
                                    deviceSymbolicLink.c_str());
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set symbolic link. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 创建媒体源
    ComPtr<IMFMediaSource> mediaSource;
    hr = MFCreateDeviceSource(sourceAttributes.Get(), &mediaSource);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create device source. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 创建源读取器属性
    ComPtr<IMFAttributes> readerAttributes;
    hr = MFCreateAttributes(&readerAttributes, 3);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create reader attributes. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 启用视频处理
    hr = readerAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_VIDEO_PROCESSING, TRUE);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to enable video processing. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 设置D3D管理器以启用硬件加速
    hr = readerAttributes->SetUnknown(MF_SOURCE_READER_D3D_MANAGER, m_dxgiManager.Get());
    if (FAILED(hr)) {
        std::wcout << L"Warning: Failed to set D3D manager, falling back to software processing" << std::endl;
    } else {
        std::wcout << L"D3D manager set successfully for hardware acceleration" << std::endl;
    }
    
    // 启用异步模式
    hr = readerAttributes->SetUINT32(MF_SOURCE_READER_ENABLE_ADVANCED_VIDEO_PROCESSING, TRUE);
    if (FAILED(hr)) {
        std::wcout << L"Warning: Failed to enable advanced video processing" << std::endl;
    }
    
    // 创建回调对象（在创建源读取器之前）
    m_readerCallback = new WMFSourceReaderCallback(this);
    
    // 设置异步回调到属性中
    hr = readerAttributes->SetUnknown(MF_SOURCE_READER_ASYNC_CALLBACK, m_readerCallback);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set async callback. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 创建源读取器
    hr = MFCreateSourceReaderFromMediaSource(mediaSource.Get(), readerAttributes.Get(), &m_sourceReader);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create source reader. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    std::wcout << L"Source reader created successfully" << std::endl;
    return true;
}

bool WMFCapture::ConfigureSourceReader() {
    HRESULT hr;
    
    // 创建输出媒体类型
    ComPtr<IMFMediaType> mediaType;
    hr = MFCreateMediaType(&mediaType);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to create media type. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 设置主要类型为视频
    hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set major type. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 优先使用NV12格式（GPU友好）
    hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
    if (FAILED(hr)) {
        std::wcout << L"NV12 not supported, trying RGB32..." << std::endl;
        hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_RGB32);
        if (FAILED(hr)) {
            std::wcerr << L"Failed to set subtype. HRESULT: " << std::hex << hr << std::endl;
            return false;
        }
    }
    
    // 设置输出媒体类型
    hr = m_sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
                                           nullptr, mediaType.Get());
    if (FAILED(hr)) {
        std::wcerr << L"Failed to set media type. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    // 获取实际的媒体类型
    ComPtr<IMFMediaType> actualMediaType;
    hr = m_sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM, &actualMediaType);
    if (SUCCEEDED(hr)) {
        GUID subtype;
        if (SUCCEEDED(actualMediaType->GetGUID(MF_MT_SUBTYPE, &subtype))) {
            if (subtype == MFVideoFormat_NV12) {
                std::wcout << L"Using NV12 format (GPU optimized)" << std::endl;
            } else if (subtype == MFVideoFormat_RGB32) {
                std::wcout << L"Using RGB32 format" << std::endl;
            } else {
                std::wcout << L"Using unknown format" << std::endl;
            }
        }
        
        UINT32 width, height;
        if (SUCCEEDED(MFGetAttributeSize(actualMediaType.Get(), MF_MT_FRAME_SIZE, &width, &height))) {
            std::wcout << L"Frame size: " << width << L"x" << height << std::endl;
        }
    }
    
    std::wcout << L"Source reader configured successfully" << std::endl;
    return true;
}

void WMFCapture::OnFrameReceived(IMFSample* sample) {
    if (!m_capturing || !m_frameCallback || !sample) {
        return;
    }
    
    std::lock_guard<std::mutex> lock(m_frameMutex);
    
    // 首先尝试作为GPU纹理处理
    if (ProcessSampleAsTexture(sample, m_currentFrame)) {
        static int textureFrameCount = 0;
        textureFrameCount++;
        if (textureFrameCount <= 3) {
            std::wcout << L"Frame " << textureFrameCount << L" processed as GPU texture (zero-copy)" << std::endl;
        }
        m_frameCallback(m_currentFrame);
    }
    // 回退到CPU处理
    else if (ProcessSampleAsCPU(sample, m_currentFrame)) {
        static int cpuFrameCount = 0;
        cpuFrameCount++;
        if (cpuFrameCount <= 3) {
            std::wcout << L"Frame " << cpuFrameCount << L" processed as CPU data" << std::endl;
        }
        m_frameCallback(m_currentFrame);
    }
}

bool WMFCapture::ProcessSampleAsTexture(IMFSample* sample, WMFFrameData& frameData) {
    HRESULT hr;
    
    // 获取DXGI缓冲区
    ComPtr<IMFMediaBuffer> mediaBuffer;
    hr = sample->GetBufferByIndex(0, &mediaBuffer);
    if (FAILED(hr)) return false;
    
    ComPtr<IMFDXGIBuffer> dxgiBuffer;
    hr = mediaBuffer->QueryInterface(IID_PPV_ARGS(&dxgiBuffer));
    if (FAILED(hr)) return false;
    
    // 获取D3D11纹理
    ComPtr<ID3D11Texture2D> texture;
    UINT subresource;
    hr = dxgiBuffer->GetResource(IID_PPV_ARGS(&texture));
    if (FAILED(hr)) return false;
    
    hr = dxgiBuffer->GetSubresourceIndex(&subresource);
    if (FAILED(hr)) return false;
    
    // 获取纹理描述
    D3D11_TEXTURE2D_DESC desc;
    texture->GetDesc(&desc);
    
    // 获取时间戳
    LONGLONG timestamp;
    hr = sample->GetSampleTime(&timestamp);
    if (FAILED(hr)) timestamp = 0;
    
    // 填充帧数据
    frameData.texture = texture;
    frameData.width = desc.Width;
    frameData.height = desc.Height;
    frameData.format = desc.Format;
    frameData.timestamp = timestamp;
    frameData.isGPUTexture = true;
    frameData.cpuData.clear(); // GPU纹理不需要CPU数据
    
    return true;
}

bool WMFCapture::ProcessSampleAsCPU(IMFSample* sample, WMFFrameData& frameData) {
    HRESULT hr;
    
    ComPtr<IMFMediaBuffer> mediaBuffer;
    hr = sample->GetBufferByIndex(0, &mediaBuffer);
    if (FAILED(hr)) return false;
    
    BYTE* data = nullptr;
    DWORD length = 0;
    
    hr = mediaBuffer->Lock(&data, nullptr, &length);
    if (FAILED(hr)) return false;
    
    // 获取时间戳
    LONGLONG timestamp;
    hr = sample->GetSampleTime(&timestamp);
    if (FAILED(hr)) timestamp = 0;
    
    // 复制数据到CPU缓冲区
    frameData.cpuData.assign(data, data + length);
    frameData.width = 1920;  // 需要从媒体类型获取实际尺寸
    frameData.height = 1080;
    frameData.format = DXGI_FORMAT_B8G8R8A8_UNORM; // 默认格式
    frameData.timestamp = timestamp;
    frameData.isGPUTexture = false;
    frameData.texture.Reset();
    
    mediaBuffer->Unlock();
    return true;
}

// WMFSourceReaderCallback实现
WMFSourceReaderCallback::WMFSourceReaderCallback(WMFCapture* parent)
    : m_parent(parent), m_refCount(1) {
}

WMFSourceReaderCallback::~WMFSourceReaderCallback() {
}

STDMETHODIMP WMFSourceReaderCallback::QueryInterface(REFIID riid, void** ppv) {
    if (riid == IID_IUnknown || riid == IID_IMFSourceReaderCallback) {
        *ppv = static_cast<IMFSourceReaderCallback*>(this);
        AddRef();
        return S_OK;
    }
    return E_NOINTERFACE;
}

STDMETHODIMP_(ULONG) WMFSourceReaderCallback::AddRef() {
    return InterlockedIncrement(&m_refCount);
}

STDMETHODIMP_(ULONG) WMFSourceReaderCallback::Release() {
    ULONG count = InterlockedDecrement(&m_refCount);
    if (count == 0) {
        delete this;
    }
    return count;
}

STDMETHODIMP WMFSourceReaderCallback::OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, 
                                                  DWORD dwStreamFlags, LONGLONG llTimestamp, 
                                                  IMFSample* pSample) {
    if (SUCCEEDED(hrStatus) && pSample && m_parent && m_parent->IsCapturing()) {
        m_parent->OnFrameReceived(pSample);
        
        // 继续读取下一帧
        HRESULT hr = m_parent->m_sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM, 
                                                         0, nullptr, nullptr, nullptr, nullptr);
        if (FAILED(hr)) {
            std::wcerr << L"Failed to continue reading samples. HRESULT: " << std::hex << hr << std::endl;
        }
    }
    
    return S_OK;
}

STDMETHODIMP WMFSourceReaderCallback::OnFlush(DWORD dwStreamIndex) {
    std::wcout << L"WMF stream flushed" << std::endl;
    return S_OK;
}

STDMETHODIMP WMFSourceReaderCallback::OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent) {
    return S_OK;
}
