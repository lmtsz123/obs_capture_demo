#pragma once

#include <windows.h>
#include <dshow.h>
#include <vector>
#include <string>

#pragma comment(lib, "strmiids.lib")
#pragma comment(lib, "ole32.lib")
#pragma comment(lib, "oleaut32.lib")

struct DirectShowDevice {
    std::string friendlyName;
    std::string devicePath;
    std::string description;
    bool isOBSVirtualCamera;
    CLSID clsid;
};

class DirectShowDeviceEnumerator {
public:
    DirectShowDeviceEnumerator();
    ~DirectShowDeviceEnumerator();

    bool Initialize();
    void Cleanup();
    
    std::vector<DirectShowDevice> EnumerateVideoDevices();
    DirectShowDevice* FindOBSVirtualCamera();
    
private:
    bool m_initialized;
    ICreateDevEnum* m_deviceEnum;
    
    std::string WStringToString(const std::wstring& wstr);
    bool IsOBSVirtualCamera(const std::string& friendlyName, const std::string& description);
    bool GetDeviceInfo(IMoniker* moniker, DirectShowDevice& device);
};
