#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <d3d11.h>
#include <dxgi1_2.h>
#include <wrl/client.h>
#include <functional>
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <atomic>

using Microsoft::WRL::ComPtr;

#pragma comment(lib, "mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#pragma comment(lib, "d3d11.lib")
#pragma comment(lib, "dxgi.lib")

// WMF设备信息
struct WMFDevice {
    std::wstring friendlyName;
    std::wstring symbolicLink;
    bool isHardwareAccelerated;
};

// WMF帧数据结构
struct WMFFrameData {
    ComPtr<ID3D11Texture2D> texture;      // GPU纹理（如果可用）
    std::vector<uint8_t> cpuData;         // CPU数据（后备方案）
    int width;
    int height;
    DXGI_FORMAT format;
    uint64_t timestamp;
    bool isGPUTexture;                     // 标记是否为GPU纹理
};

// 前向声明
class WMFSourceReaderCallback;

class WMFCapture {
public:
    using FrameCallback = std::function<void(const WMFFrameData&)>;

    WMFCapture();
    ~WMFCapture();

    bool Initialize();
    void Cleanup();
    
    // 设备枚举
    std::vector<WMFDevice> EnumerateDevices();
    
    // 启动采集
    bool StartCapture(const std::wstring& deviceSymbolicLink, FrameCallback callback);
    void StopCapture();
    
    bool IsCapturing() const { return m_capturing; }
    
    // D3D11设备管理
    bool InitializeD3D11();
    ID3D11Device* GetD3D11Device() const { return m_d3d11Device.Get(); }

private:
    bool m_initialized;
    std::atomic<bool> m_capturing;
    FrameCallback m_frameCallback;
    
    // WMF组件
    ComPtr<IMFSourceReader> m_sourceReader;
    ComPtr<IMFDXGIDeviceManager> m_dxgiManager;
    WMFSourceReaderCallback* m_readerCallback;
    
    // D3D11组件
    ComPtr<ID3D11Device> m_d3d11Device;
    ComPtr<ID3D11DeviceContext> m_d3d11Context;
    UINT m_resetToken;
    
    // 帧数据
    std::mutex m_frameMutex;
    WMFFrameData m_currentFrame;
    
    bool CreateSourceReader(const std::wstring& deviceSymbolicLink);
    bool ConfigureSourceReader();
    void OnFrameReceived(IMFSample* sample);
    bool ProcessSampleAsTexture(IMFSample* sample, WMFFrameData& frameData);
    bool ProcessSampleAsCPU(IMFSample* sample, WMFFrameData& frameData);
    
    friend class WMFSourceReaderCallback;
};

// WMF Source Reader回调类
class WMFSourceReaderCallback : public IMFSourceReaderCallback {
public:
    WMFSourceReaderCallback(WMFCapture* parent);
    virtual ~WMFSourceReaderCallback();

    // IUnknown
    STDMETHODIMP QueryInterface(REFIID riid, void** ppv) override;
    STDMETHODIMP_(ULONG) AddRef() override;
    STDMETHODIMP_(ULONG) Release() override;

    // IMFSourceReaderCallback
    STDMETHODIMP OnReadSample(HRESULT hrStatus, DWORD dwStreamIndex, 
                             DWORD dwStreamFlags, LONGLONG llTimestamp, 
                             IMFSample* pSample) override;
    STDMETHODIMP OnFlush(DWORD dwStreamIndex) override;
    STDMETHODIMP OnEvent(DWORD dwStreamIndex, IMFMediaEvent* pEvent) override;

private:
    WMFCapture* m_parent;
    LONG m_refCount;
};
