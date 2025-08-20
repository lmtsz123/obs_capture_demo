#include "camera_capture.h"
#include <iostream>
#include <comdef.h>
#include <mferror.h>
#include <mfreadwrite.h>
#include <mfapi.h>

// 如果常量未定义，手动定义
#ifndef MF_SOURCE_READER_ENDOFSTREAM
#define MF_SOURCE_READER_ENDOFSTREAM 0x00000002
#endif

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "strmiids.lib")

class CameraCapture::MediaFoundationReader {
public:
    IMFSourceReader* sourceReader = nullptr;
    FrameCallback frameCallback;
    bool running = false;
    HANDLE captureThread = nullptr;
    
    static DWORD WINAPI CaptureThreadProc(LPVOID param) {
        MediaFoundationReader* reader = static_cast<MediaFoundationReader*>(param);
        reader->CaptureLoop();
        return 0;
    }
    
    void CaptureLoop() {
        HRESULT hr;
        while (running) {
            IMFSample* sample = nullptr;
            DWORD streamFlags = 0;
            LONGLONG timestamp = 0;
            
            hr = sourceReader->ReadSample(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                        0, nullptr, &streamFlags, &timestamp, &sample);
            
            if (SUCCEEDED(hr) && sample) {
                ProcessSample(sample);
                sample->Release();
            }
            
            if (streamFlags & MF_SOURCE_READER_ENDOFSTREAM) {
                break;
            }
            
            Sleep(16); // ~60fps
        }
    }
    
    void ProcessSample(IMFSample* sample) {
        IMFMediaBuffer* buffer = nullptr;
        HRESULT hr = sample->GetBufferByIndex(0, &buffer);
        
        if (SUCCEEDED(hr)) {
            BYTE* data = nullptr;
            DWORD length = 0;
            
            hr = buffer->Lock(&data, nullptr, &length);
            if (SUCCEEDED(hr)) {
                // 假设这里得到的是NV12格式的数据
                FrameData frameData;
                frameData.data = data;
                frameData.size = length;
                frameData.width = 1920; // 需要从媒体类型获取实际尺寸
                frameData.height = 1080;
                frameData.format = MFVideoFormat_NV12;
                
                if (frameCallback) {
                    frameCallback(frameData);
                }
                
                buffer->Unlock();
            }
            buffer->Release();
        }
    }
};

CameraCapture::CameraCapture() : m_reader(nullptr), m_initialized(false), m_capturing(false) {
    m_reader = new MediaFoundationReader();
}

CameraCapture::~CameraCapture() {
    Cleanup();
    delete m_reader;
}

bool CameraCapture::Initialize() {
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr)) return false;
    
    hr = MFStartup(MF_VERSION);
    if (FAILED(hr)) {
        CoUninitialize();
        return false;
    }
    
    // 初始化DirectShow枚举器
    if (!m_dshowEnum.Initialize()) {
        std::cerr << "Failed to initialize DirectShow enumerator" << std::endl;
    }
    
    m_initialized = true;
    return true;
}

std::vector<std::string> CameraCapture::GetDeviceList() {
    std::vector<std::string> devices;
    
    IMFAttributes* attributes = nullptr;
    IMFActivate** deviceArray = nullptr;
    UINT32 deviceCount = 0;
    
    HRESULT hr = MFCreateAttributes(&attributes, 1);
    if (FAILED(hr)) return devices;
    
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    
    if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(attributes, &deviceArray, &deviceCount);
        
        if (SUCCEEDED(hr)) {
            for (UINT32 i = 0; i < deviceCount; i++) {
                WCHAR* friendlyName = nullptr;
                UINT32 nameLength = 0;
                
                hr = deviceArray[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                                      &friendlyName, &nameLength);
                
                if (SUCCEEDED(hr)) {
                    // 正确转换宽字符串到多字节字符串
                    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, nullptr, 0, nullptr, nullptr);
                    if (bufferSize > 0) {
                        std::string name(bufferSize - 1, 0);
                        WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, &name[0], bufferSize, nullptr, nullptr);
                        devices.push_back(name);
                    }
                    CoTaskMemFree(friendlyName);
                }
                
                deviceArray[i]->Release();
            }
            CoTaskMemFree(deviceArray);
        }
    }
    
    if (attributes) attributes->Release();
    return devices;
}

bool CameraCapture::StartCapture(const std::string& deviceName, FrameCallback callback) {
    if (!m_initialized || m_capturing) return false;
    
    std::cout << "Attempting to start capture for device: " << deviceName << std::endl;
    
    IMFAttributes* attributes = nullptr;
    IMFActivate** deviceArray = nullptr;
    UINT32 deviceCount = 0;
    IMFMediaSource* mediaSource = nullptr;
    
    HRESULT hr = MFCreateAttributes(&attributes, 2);
    if (FAILED(hr)) return false;
    
    hr = attributes->SetGUID(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE,
                           MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_GUID);
    
    // 尝试包含虚拟设备
    if (SUCCEEDED(hr)) {
        hr = attributes->SetUINT32(MF_DEVSOURCE_ATTRIBUTE_SOURCE_TYPE_VIDCAP_CATEGORY, 
                                  PINDIR_OUTPUT);
    }
    
    if (SUCCEEDED(hr)) {
        hr = MFEnumDeviceSources(attributes, &deviceArray, &deviceCount);
        
        if (SUCCEEDED(hr)) {
            std::cout << "Media Foundation found " << deviceCount << " devices:" << std::endl;
            
            // 查找指定设备
            for (UINT32 i = 0; i < deviceCount; i++) {
                WCHAR* friendlyName = nullptr;
                UINT32 nameLength = 0;
                
                hr = deviceArray[i]->GetAllocatedString(MF_DEVSOURCE_ATTRIBUTE_FRIENDLY_NAME,
                                                      &friendlyName, &nameLength);
                
                if (SUCCEEDED(hr)) {
                    // 正确转换宽字符串到多字节字符串
                    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, nullptr, 0, nullptr, nullptr);
                    std::string name;
                    if (bufferSize > 0) {
                        name.resize(bufferSize - 1);
                        WideCharToMultiByte(CP_UTF8, 0, friendlyName, -1, &name[0], bufferSize, nullptr, nullptr);
                    }
                    
                    std::cout << "  " << i << ": " << name << std::endl;
                    
                    // 使用更灵活的匹配方式
                    if (name == deviceName || 
                        name.find(deviceName) != std::string::npos || 
                        deviceName.find(name) != std::string::npos) {
                        std::cout << "  -> Matched! Attempting to activate device." << std::endl;
                        hr = deviceArray[i]->ActivateObject(IID_PPV_ARGS(&mediaSource));
                        if (SUCCEEDED(hr)) {
                            std::cout << "  -> Device activated successfully." << std::endl;
                        } else {
                            std::cout << "  -> Failed to activate device. HRESULT: " << std::hex << hr << std::endl;
                        }
                        CoTaskMemFree(friendlyName);
                        break;
                    }
                    CoTaskMemFree(friendlyName);
                }
                deviceArray[i]->Release();
            }
            CoTaskMemFree(deviceArray);
        }
    }
    
    if (attributes) attributes->Release();
    
    if (!mediaSource) {
        std::cout << "Failed to find device in Media Foundation. Trying alternative method..." << std::endl;
        
        // 尝试使用DirectShow设备信息来创建Media Foundation源
        auto dshowDevices = m_dshowEnum.EnumerateVideoDevices();
        for (const auto& device : dshowDevices) {
            if (device.friendlyName == deviceName && !device.devicePath.empty()) {
                std::cout << "Trying to use device path: " << device.devicePath << std::endl;
                
                // 尝试使用设备路径创建源
                std::wstring wDevicePath(device.devicePath.begin(), device.devicePath.end());
                hr = MFCreateDeviceSource(attributes, &mediaSource);
                if (SUCCEEDED(hr)) {
                    std::cout << "Successfully created device source using alternative method." << std::endl;
                    break;
                }
            }
        }
        
        if (!mediaSource) {
            std::cout << "All methods failed to find or activate media source for device: " << deviceName << std::endl;
            return false;
        }
    }
    
    std::cout << "Creating source reader..." << std::endl;
    
    // 创建源读取器
    hr = MFCreateSourceReaderFromMediaSource(mediaSource, nullptr, &m_reader->sourceReader);
    mediaSource->Release();
    
    if (FAILED(hr)) {
        std::cout << "Failed to create source reader. HRESULT: " << std::hex << hr << std::endl;
        return false;
    }
    
    std::cout << "Source reader created successfully." << std::endl;
    
    // 配置媒体类型为NV12
    IMFMediaType* mediaType = nullptr;
    hr = MFCreateMediaType(&mediaType);
    if (SUCCEEDED(hr)) {
        hr = mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Video);
        if (SUCCEEDED(hr)) {
            hr = mediaType->SetGUID(MF_MT_SUBTYPE, MFVideoFormat_NV12);
        }
        if (SUCCEEDED(hr)) {
            hr = m_reader->sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_VIDEO_STREAM,
                                                            nullptr, mediaType);
        }
        mediaType->Release();
    }
    
    m_reader->frameCallback = callback;
    m_reader->running = true;
    m_reader->captureThread = CreateThread(nullptr, 0, 
                                         MediaFoundationReader::CaptureThreadProc,
                                         m_reader, 0, nullptr);
    
    m_capturing = true;
    return true;
}

void CameraCapture::StopCapture() {
    if (!m_capturing) return;
    
    m_reader->running = false;
    if (m_reader->captureThread) {
        WaitForSingleObject(m_reader->captureThread, INFINITE);
        CloseHandle(m_reader->captureThread);
        m_reader->captureThread = nullptr;
    }
    
    if (m_reader->sourceReader) {
        m_reader->sourceReader->Release();
        m_reader->sourceReader = nullptr;
    }
    
    m_capturing = false;
}

void CameraCapture::Cleanup() {
    StopCapture();
    
    if (m_initialized) {
        m_dshowEnum.Cleanup();
        MFShutdown();
        CoUninitialize();
        m_initialized = false;
    }
}

std::vector<DirectShowDevice> CameraCapture::GetDirectShowDevices() {
    return m_dshowEnum.EnumerateVideoDevices();
}

DirectShowDevice* CameraCapture::FindOBSVirtualCamera() {
    return m_dshowEnum.FindOBSVirtualCamera();
}