#pragma once

#include <windows.h>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#include <dshow.h>
#include <vector>
#include <string>
#include <functional>
#include "dshow_device_enum.h"

class CameraCapture {
public:
    struct FrameData {
        uint8_t* data;
        size_t size;
        int width;
        int height;
        GUID format;
    };

    using FrameCallback = std::function<void(const FrameData&)>;

    CameraCapture();
    ~CameraCapture();

    bool Initialize();
    bool StartCapture(const std::string& deviceName, FrameCallback callback);
    void StopCapture();
    void Cleanup();

    std::vector<std::string> GetDeviceList();
    
    // DirectShow设备枚举方法
    std::vector<DirectShowDevice> GetDirectShowDevices();
    DirectShowDevice* FindOBSVirtualCamera();

private:
    class MediaFoundationReader;
    MediaFoundationReader* m_reader;
    DirectShowDeviceEnumerator m_dshowEnum;
    bool m_initialized;
    bool m_capturing;
};