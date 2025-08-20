#include "dshow_device_enum.h"
#include <iostream>
#include <comdef.h>
#include <algorithm>
#include <cctype>

DirectShowDeviceEnumerator::DirectShowDeviceEnumerator() 
    : m_initialized(false), m_deviceEnum(nullptr) {
}

DirectShowDeviceEnumerator::~DirectShowDeviceEnumerator() {
    Cleanup();
}

bool DirectShowDeviceEnumerator::Initialize() {
    if (m_initialized) return true;
    
    HRESULT hr = CoInitializeEx(nullptr, COINIT_APARTMENTTHREADED);
    if (FAILED(hr) && hr != RPC_E_CHANGED_MODE) {
        std::cerr << "Failed to initialize COM" << std::endl;
        return false;
    }
    
    // 创建设备枚举器
    hr = CoCreateInstance(CLSID_SystemDeviceEnum, nullptr, CLSCTX_INPROC_SERVER,
                         IID_ICreateDevEnum, (void**)&m_deviceEnum);
    
    if (FAILED(hr)) {
        std::cerr << "Failed to create device enumerator: " << std::hex << hr << std::endl;
        return false;
    }
    
    m_initialized = true;
    return true;
}

void DirectShowDeviceEnumerator::Cleanup() {
    if (m_deviceEnum) {
        m_deviceEnum->Release();
        m_deviceEnum = nullptr;
    }
    m_initialized = false;
}

std::vector<DirectShowDevice> DirectShowDeviceEnumerator::EnumerateVideoDevices() {
    std::vector<DirectShowDevice> devices;
    
    if (!m_initialized) {
        std::cerr << "DirectShow enumerator not initialized" << std::endl;
        return devices;
    }
    
    IEnumMoniker* enumMoniker = nullptr;
    HRESULT hr = m_deviceEnum->CreateClassEnumerator(CLSID_VideoInputDeviceCategory, 
                                                    &enumMoniker, 0);
    
    if (hr != S_OK) {
        std::cerr << "No video capture devices found" << std::endl;
        return devices;
    }
    
    IMoniker* moniker = nullptr;
    while (enumMoniker->Next(1, &moniker, nullptr) == S_OK) {
        DirectShowDevice device;
        if (GetDeviceInfo(moniker, device)) {
            devices.push_back(device);
        }
        moniker->Release();
    }
    
    enumMoniker->Release();
    return devices;
}

DirectShowDevice* DirectShowDeviceEnumerator::FindOBSVirtualCamera() {
    static DirectShowDevice obsDevice;
    auto devices = EnumerateVideoDevices();
    
    // 首先查找完整名称的"OBS Virtual Camera"
    for (auto& device : devices) {
        if (device.friendlyName == "OBS Virtual Camera") {
            obsDevice = device;
            return &obsDevice;
        }
    }
    
    // 如果没找到，再查找其他OBS相关设备
    for (auto& device : devices) {
        if (device.isOBSVirtualCamera) {
            obsDevice = device;
            return &obsDevice;
        }
    }
    
    return nullptr;
}

bool DirectShowDeviceEnumerator::GetDeviceInfo(IMoniker* moniker, DirectShowDevice& device) {
    IPropertyBag* propertyBag = nullptr;
    HRESULT hr = moniker->BindToStorage(nullptr, nullptr, IID_IPropertyBag, 
                                       (void**)&propertyBag);
    
    if (FAILED(hr)) return false;
    
    VARIANT varName, varDescription, varDevicePath, varCLSID;
    VariantInit(&varName);
    VariantInit(&varDescription);
    VariantInit(&varDevicePath);
    VariantInit(&varCLSID);
    
    // 获取友好名称
    hr = propertyBag->Read(L"FriendlyName", &varName, nullptr);
    if (SUCCEEDED(hr) && varName.vt == VT_BSTR) {
        device.friendlyName = WStringToString(std::wstring(varName.bstrVal));
    }
    
    // 获取描述
    hr = propertyBag->Read(L"Description", &varDescription, nullptr);
    if (SUCCEEDED(hr) && varDescription.vt == VT_BSTR) {
        device.description = WStringToString(std::wstring(varDescription.bstrVal));
    }
    
    // 获取设备路径
    hr = propertyBag->Read(L"DevicePath", &varDevicePath, nullptr);
    if (SUCCEEDED(hr) && varDevicePath.vt == VT_BSTR) {
        device.devicePath = WStringToString(std::wstring(varDevicePath.bstrVal));
    }
    
    // 获取CLSID
    hr = propertyBag->Read(L"CLSID", &varCLSID, nullptr);
    if (SUCCEEDED(hr) && varCLSID.vt == VT_BSTR) {
        CLSIDFromString(varCLSID.bstrVal, &device.clsid);
    }
    
    // 检查是否是OBS Virtual Camera
    device.isOBSVirtualCamera = IsOBSVirtualCamera(device.friendlyName, device.description);
    
    VariantClear(&varName);
    VariantClear(&varDescription);
    VariantClear(&varDevicePath);
    VariantClear(&varCLSID);
    propertyBag->Release();
    
    return true;
}

bool DirectShowDeviceEnumerator::IsOBSVirtualCamera(const std::string& friendlyName, 
                                                   const std::string& description) {
    // 将字符串转换为小写进行比较
    std::string lowerName = friendlyName;
    std::string lowerDesc = description;
    
    std::transform(lowerName.begin(), lowerName.end(), lowerName.begin(), ::tolower);
    std::transform(lowerDesc.begin(), lowerDesc.end(), lowerDesc.begin(), ::tolower);
    
    // 精确匹配"OBS Virtual Camera"
    if (friendlyName == "OBS Virtual Camera" || lowerName == "obs virtual camera") {
        return true;
    }
    
    // OBS Virtual Camera的其他可能标识
    std::vector<std::string> obsIdentifiers = {
        "obs virtual camera",
        "obs-camera", 
        "obs camera",
        "obs virtual cam",
        "obs studio",
        "virtual camera obs"
    };
    
    // 检查友好名称
    for (const auto& identifier : obsIdentifiers) {
        if (lowerName.find(identifier) != std::string::npos) {
            return true;
        }
    }
    
    // 检查描述
    for (const auto& identifier : obsIdentifiers) {
        if (lowerDesc.find(identifier) != std::string::npos) {
            return true;
        }
    }
    
    // 检查特定的OBS Virtual Camera模式
    if (lowerName.find("virtual") != std::string::npos && 
        (lowerName.find("camera") != std::string::npos || lowerName.find("cam") != std::string::npos)) {
        // 进一步检查是否可能是OBS
        if (lowerDesc.find("obs") != std::string::npos || 
            lowerName.find("obs") != std::string::npos) {
            return true;
        }
    }
    
    return false;
}

std::string DirectShowDeviceEnumerator::WStringToString(const std::wstring& wstr) {
    if (wstr.empty()) return std::string();
    
    int bufferSize = WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
    if (bufferSize <= 0) return std::string();
    
    std::string result(bufferSize - 1, 0);
    WideCharToMultiByte(CP_UTF8, 0, wstr.c_str(), -1, &result[0], bufferSize, nullptr, nullptr);
    
    return result;
}
