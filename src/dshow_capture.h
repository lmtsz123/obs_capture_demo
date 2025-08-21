#pragma once

#include <windows.h>
#include <dshow.h>
#include "qedit.h"
#include <atlbase.h>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d11.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")
#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "d3d11.lib")

// 前向声明
class SampleGrabberCallback;

struct DirectShowFrameData {
    std::vector<uint8_t> data;
    int width;
    int height;
    GUID format;
    uint64_t timestamp;
};

class DirectShowCapture {
public:
    using FrameCallback = std::function<void(const DirectShowFrameData&)>;

    DirectShowCapture();
    ~DirectShowCapture();

    bool Initialize();
    void Cleanup();
    
    bool StartCapture(const std::string& deviceName, FrameCallback callback);
    void StopCapture();
    
    bool IsCapturing() const { return m_capturing; }

private:
    bool m_initialized;
    std::atomic<bool> m_capturing;
    FrameCallback m_frameCallback;
    
    // DirectShow组件
    CComPtr<IGraphBuilder> m_graphBuilder;
    CComPtr<ICaptureGraphBuilder2> m_captureBuilder;
    CComPtr<IMediaControl> m_mediaControl;
    CComPtr<IBaseFilter> m_sourceFilter;
    CComPtr<IBaseFilter> m_sampleGrabber;
    CComPtr<IBaseFilter> m_nullRenderer;
    
    // 回调处理
    SampleGrabberCallback* m_grabberCallback;
    
    // 帧数据
    std::mutex m_frameMutex;
    DirectShowFrameData m_currentFrame;
    
    bool CreateCaptureGraph();
    bool FindCaptureDevice(const std::string& deviceName, IBaseFilter** sourceFilter);
    bool ConfigureGraph();
    bool SetupSampleGrabber();
    void OnFrameReceived(const uint8_t* buffer, long bufferSize, double sampleTime);
    
    friend class SampleGrabberCallback;
};

// SampleGrabber回调类
class SampleGrabberCallback : public ISampleGrabberCB {
public:
    SampleGrabberCallback(DirectShowCapture* parent);
    virtual ~SampleGrabberCallback();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv);
    STDMETHODIMP_(ULONG) AddRef();
    STDMETHODIMP_(ULONG) Release();

    // ISampleGrabberCB
    STDMETHODIMP SampleCB(double sampleTime, IMediaSample* sample);
    STDMETHODIMP BufferCB(double sampleTime, BYTE* buffer, long bufferLen);

private:
    DirectShowCapture* m_parent;
    LONG m_refCount;
    
    // D3D11纹理处理方法
    bool TryProcessAsDirect3D11(IMediaSample* sample, double timestamp);
};
