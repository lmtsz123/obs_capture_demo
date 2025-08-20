#include "d3d11_renderer.h"
#include <iostream>
#include <vector>

// HLSL Vertex Shader
const char* g_vertexShaderSource = R"(
struct VS_INPUT {
    float2 pos : POSITION;
    float2 tex : TEXCOORD0;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

PS_INPUT main(VS_INPUT input) {
    PS_INPUT output;
    output.pos = float4(input.pos, 0.0f, 1.0f);
    output.tex = input.tex;
    return output;
}
)";

// HLSL Pixel Shader for NV12 to RGBA conversion
const char* g_pixelShaderSource = R"(
Texture2D yTexture : register(t0);
Texture2D uvTexture : register(t1);
SamplerState texSampler : register(s0);

cbuffer Constants : register(b0) {
    float textureWidth;
    float textureHeight;
    float2 padding;
};

struct PS_INPUT {
    float4 pos : SV_POSITION;
    float2 tex : TEXCOORD0;
};

float4 main(PS_INPUT input) : SV_Target {
    float y = yTexture.Sample(texSampler, input.tex).r;
    float2 uv = uvTexture.Sample(texSampler, input.tex).rg;
    
    // NV12 to RGB conversion (BT.709)
    float u = uv.r - 0.5f;
    float v = uv.g - 0.5f;
    
    float r = y + 1.5748f * v;
    float g = y - 0.1873f * u - 0.4681f * v;
    float b = y + 1.8556f * u;
    
    return float4(saturate(r), saturate(g), saturate(b), 1.0f);
}
)";

D3D11Renderer::D3D11Renderer() {
}

D3D11Renderer::~D3D11Renderer() {
    Cleanup();
}

bool D3D11Renderer::Initialize() {
    // 默认初始化，不创建SwapChain（用于离屏渲染）
    if (!CreateDevice()) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
        return false;
    }
    
    if (!CreateShaders()) {
        std::cerr << "Failed to create shaders" << std::endl;
        return false;
    }
    
    if (!CreateBuffers()) {
        std::cerr << "Failed to create buffers" << std::endl;
        return false;
    }
    
    return true;
}

bool D3D11Renderer::Initialize(void* windowHandle) {
    if (!CreateDevice()) {
        std::cerr << "Failed to create D3D11 device" << std::endl;
        return false;
    }
    
    if (!CreateSwapChain(windowHandle)) {
        std::cerr << "Failed to create D3D11 swap chain" << std::endl;
        return false;
    }
    
    if (!CreateShaders()) {
        std::cerr << "Failed to create shaders" << std::endl;
        return false;
    }
    
    if (!CreateBuffers()) {
        std::cerr << "Failed to create buffers" << std::endl;
        return false;
    }
    
    return true;
}

void D3D11Renderer::Cleanup() {
    // ComPtr会自动释放资源
    m_device.Reset();
    m_context.Reset();
    m_swapChain.Reset();
    m_renderTargetView.Reset();
    m_vertexShader.Reset();
    m_pixelShader.Reset();
    m_inputLayout.Reset();
    m_yTexture.Reset();
    m_uvTexture.Reset();
    m_ySRV.Reset();
    m_uvSRV.Reset();
    m_samplerState.Reset();
    m_vertexBuffer.Reset();
    m_indexBuffer.Reset();
    m_constantBuffer.Reset();
    m_offscreenTexture.Reset();
    m_offscreenRTV.Reset();
    m_stagingTexture.Reset();
}

bool D3D11Renderer::CreateDevice() {
    HRESULT hr;
    
    // 创建设备和设备上下文
    D3D_FEATURE_LEVEL featureLevels[] = {
        D3D_FEATURE_LEVEL_11_1,
        D3D_FEATURE_LEVEL_11_0,
        D3D_FEATURE_LEVEL_10_1,
        D3D_FEATURE_LEVEL_10_0
    };
    
    UINT createDeviceFlags = 0;
#ifdef _DEBUG
    createDeviceFlags |= D3D11_CREATE_DEVICE_DEBUG;
#endif
    
    hr = D3D11CreateDevice(
        nullptr,                    // 使用默认适配器
        D3D_DRIVER_TYPE_HARDWARE,   // 硬件驱动
        nullptr,                    // 软件驱动句柄
        createDeviceFlags,          // 创建标志
        featureLevels,              // 特性级别数组
        ARRAYSIZE(featureLevels),   // 特性级别数量
        D3D11_SDK_VERSION,          // SDK版本
        &m_device,                  // 输出设备
        nullptr,                    // 输出特性级别
        &m_context                  // 输出设备上下文
    );
    
    return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateSwapChain(void* windowHandle) {
    HRESULT hr;
    
    // 获取DXGI工厂
    ComPtr<IDXGIFactory1> dxgiFactory;
    ComPtr<IDXGIDevice> dxgiDevice;
    ComPtr<IDXGIAdapter> dxgiAdapter;
    
    hr = m_device->QueryInterface(__uuidof(IDXGIDevice), (void**)&dxgiDevice);
    if (FAILED(hr)) return false;
    
    hr = dxgiDevice->GetAdapter(&dxgiAdapter);
    if (FAILED(hr)) return false;
    
    hr = dxgiAdapter->GetParent(__uuidof(IDXGIFactory1), (void**)&dxgiFactory);
    if (FAILED(hr)) return false;
    
    // 配置SwapChain描述
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = 1;
    swapChainDesc.BufferDesc.Width = 1080;  // 使用视频尺寸
    swapChainDesc.BufferDesc.Height = 1920;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferDesc.RefreshRate.Numerator = 60;
    swapChainDesc.BufferDesc.RefreshRate.Denominator = 1;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.OutputWindow = (HWND)windowHandle;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.SampleDesc.Quality = 0;
    swapChainDesc.Windowed = TRUE;
    
    hr = dxgiFactory->CreateSwapChain(m_device.Get(), &swapChainDesc, &m_swapChain);
    if (FAILED(hr)) return false;
    
    // 创建渲染目标视图
    ComPtr<ID3D11Texture2D> backBuffer;
    hr = m_swapChain->GetBuffer(0, __uuidof(ID3D11Texture2D), (void**)&backBuffer);
    if (FAILED(hr)) return false;
    
    hr = m_device->CreateRenderTargetView(backBuffer.Get(), nullptr, &m_renderTargetView);
    if (FAILED(hr)) return false;
    
    std::cout << "D3D11 SwapChain and RenderTargetView created successfully" << std::endl;
    return true;
}

bool D3D11Renderer::CreateShaders() {
    HRESULT hr;
    ComPtr<ID3DBlob> vsBlob, psBlob, errorBlob;
    
    // 编译顶点着色器
    hr = D3DCompile(
        g_vertexShaderSource,
        strlen(g_vertexShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "vs_5_0",
        0,
        0,
        &vsBlob,
        &errorBlob
    );
    
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Vertex shader compilation error: " 
                      << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    // 编译像素着色器
    hr = D3DCompile(
        g_pixelShaderSource,
        strlen(g_pixelShaderSource),
        nullptr,
        nullptr,
        nullptr,
        "main",
        "ps_5_0",
        0,
        0,
        &psBlob,
        &errorBlob
    );
    
    if (FAILED(hr)) {
        if (errorBlob) {
            std::cerr << "Pixel shader compilation error: " 
                      << (char*)errorBlob->GetBufferPointer() << std::endl;
        }
        return false;
    }
    
    // 创建着色器对象
    hr = m_device->CreateVertexShader(
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        nullptr,
        &m_vertexShader
    );
    if (FAILED(hr)) return false;
    
    hr = m_device->CreatePixelShader(
        psBlob->GetBufferPointer(),
        psBlob->GetBufferSize(),
        nullptr,
        &m_pixelShader
    );
    if (FAILED(hr)) return false;
    
    // 创建输入布局
    D3D11_INPUT_ELEMENT_DESC layout[] = {
        { "POSITION", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 0, D3D11_INPUT_PER_VERTEX_DATA, 0 },
        { "TEXCOORD", 0, DXGI_FORMAT_R32G32_FLOAT, 0, 8, D3D11_INPUT_PER_VERTEX_DATA, 0 }
    };
    
    hr = m_device->CreateInputLayout(
        layout,
        ARRAYSIZE(layout),
        vsBlob->GetBufferPointer(),
        vsBlob->GetBufferSize(),
        &m_inputLayout
    );
    
    return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateBuffers() {
    HRESULT hr;
    
    // 创建顶点缓冲区（全屏四边形）
    struct Vertex {
        float pos[2];
        float tex[2];
    };
    
    Vertex vertices[] = {
        { {-1.0f, -1.0f}, {0.0f, 1.0f} },  // 左下
        { {-1.0f,  1.0f}, {0.0f, 0.0f} },  // 左上
        { { 1.0f,  1.0f}, {1.0f, 0.0f} },  // 右上
        { { 1.0f, -1.0f}, {1.0f, 1.0f} }   // 右下
    };
    
    D3D11_BUFFER_DESC bufferDesc = {};
    bufferDesc.Usage = D3D11_USAGE_DEFAULT;
    bufferDesc.ByteWidth = sizeof(vertices);
    bufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
    
    D3D11_SUBRESOURCE_DATA initData = {};
    initData.pSysMem = vertices;
    
    hr = m_device->CreateBuffer(&bufferDesc, &initData, &m_vertexBuffer);
    if (FAILED(hr)) return false;
    
    // 创建索引缓冲区
    UINT indices[] = { 0, 1, 2, 0, 2, 3 };
    
    bufferDesc.ByteWidth = sizeof(indices);
    bufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
    initData.pSysMem = indices;
    
    hr = m_device->CreateBuffer(&bufferDesc, &initData, &m_indexBuffer);
    if (FAILED(hr)) return false;
    
    // 创建常量缓冲区
    bufferDesc.ByteWidth = sizeof(ConstantBuffer);
    bufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
    bufferDesc.Usage = D3D11_USAGE_DYNAMIC;
    bufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = m_device->CreateBuffer(&bufferDesc, nullptr, &m_constantBuffer);
    if (FAILED(hr)) return false;
    
    // 创建采样器状态
    D3D11_SAMPLER_DESC samplerDesc = {};
    samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
    samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_CLAMP;
    samplerDesc.ComparisonFunc = D3D11_COMPARISON_NEVER;
    samplerDesc.MinLOD = 0;
    samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;
    
    hr = m_device->CreateSamplerState(&samplerDesc, &m_samplerState);
    
    return SUCCEEDED(hr);
}

bool D3D11Renderer::CreateTextures(int width, int height) {
    HRESULT hr;
    
    // 创建Y纹理（亮度）
    D3D11_TEXTURE2D_DESC textureDesc = {};
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.MipLevels = 1;
    textureDesc.ArraySize = 1;
    textureDesc.Format = DXGI_FORMAT_R8_UNORM;
    textureDesc.SampleDesc.Count = 1;
    textureDesc.Usage = D3D11_USAGE_DYNAMIC;
    textureDesc.BindFlags = D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
    
    hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_yTexture);
    if (FAILED(hr)) return false;
    
    // 创建UV纹理（色度）
    textureDesc.Width = width / 2;
    textureDesc.Height = height / 2;
    textureDesc.Format = DXGI_FORMAT_R8G8_UNORM;
    
    hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_uvTexture);
    if (FAILED(hr)) return false;
    
    // 创建着色器资源视图
    D3D11_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
    srvDesc.Format = DXGI_FORMAT_R8_UNORM;
    srvDesc.ViewDimension = D3D11_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = 1;
    
    hr = m_device->CreateShaderResourceView(m_yTexture.Get(), &srvDesc, &m_ySRV);
    if (FAILED(hr)) return false;
    
    srvDesc.Format = DXGI_FORMAT_R8G8_UNORM;
    hr = m_device->CreateShaderResourceView(m_uvTexture.Get(), &srvDesc, &m_uvSRV);
    if (FAILED(hr)) return false;
    
    // 创建离屏渲染纹理
    textureDesc.Width = width;
    textureDesc.Height = height;
    textureDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    textureDesc.Usage = D3D11_USAGE_DEFAULT;
    textureDesc.BindFlags = D3D11_BIND_RENDER_TARGET | D3D11_BIND_SHADER_RESOURCE;
    textureDesc.CPUAccessFlags = 0;
    
    hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_offscreenTexture);
    if (FAILED(hr)) return false;
    
    // 创建渲染目标视图
    hr = m_device->CreateRenderTargetView(m_offscreenTexture.Get(), nullptr, &m_offscreenRTV);
    if (FAILED(hr)) return false;
    
    // 创建用于CPU读取的暂存纹理
    textureDesc.Usage = D3D11_USAGE_STAGING;
    textureDesc.BindFlags = 0;
    textureDesc.CPUAccessFlags = D3D11_CPU_ACCESS_READ;
    
    hr = m_device->CreateTexture2D(&textureDesc, nullptr, &m_stagingTexture);
    
    return SUCCEEDED(hr);
}

bool D3D11Renderer::UpdateNV12Textures(const uint8_t* nv12Data, int width, int height) {
    HRESULT hr;
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    
    // 更新Y纹理
    hr = m_context->Map(m_yTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return false;
    
    const uint8_t* srcY = nv12Data;
    uint8_t* dstY = static_cast<uint8_t*>(mappedResource.pData);
    
    for (int y = 0; y < height; ++y) {
        memcpy(dstY + y * mappedResource.RowPitch, srcY + y * width, width);
    }
    
    m_context->Unmap(m_yTexture.Get(), 0);
    
    // 更新UV纹理
    hr = m_context->Map(m_uvTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return false;
    
    const uint8_t* srcUV = nv12Data + width * height;
    uint8_t* dstUV = static_cast<uint8_t*>(mappedResource.pData);
    
    for (int y = 0; y < height / 2; ++y) {
        memcpy(dstUV + y * mappedResource.RowPitch, srcUV + y * width, width);
    }
    
    m_context->Unmap(m_uvTexture.Get(), 0);
    
    return true;
}

void D3D11Renderer::SetupRenderState() {
    // 设置视口 - 使用正确的窗口尺寸
    D3D11_VIEWPORT viewport = {};
    viewport.Width = static_cast<float>(1080);   // 窗口宽度
    viewport.Height = static_cast<float>(1920);  // 窗口高度
    viewport.MinDepth = 0.0f;
    viewport.MaxDepth = 1.0f;
    viewport.TopLeftX = 0.0f;
    viewport.TopLeftY = 0.0f;
    m_context->RSSetViewports(1, &viewport);
    
    // 设置输入布局
    m_context->IASetInputLayout(m_inputLayout.Get());
    
    // 设置顶点缓冲区
    UINT stride = sizeof(float) * 4;  // position + texcoord
    UINT offset = 0;
    m_context->IASetVertexBuffers(0, 1, m_vertexBuffer.GetAddressOf(), &stride, &offset);
    m_context->IASetIndexBuffer(m_indexBuffer.Get(), DXGI_FORMAT_R32_UINT, 0);
    m_context->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    
    // 设置着色器
    m_context->VSSetShader(m_vertexShader.Get(), nullptr, 0);
    m_context->PSSetShader(m_pixelShader.Get(), nullptr, 0);
    
    // 设置采样器
    m_context->PSSetSamplers(0, 1, m_samplerState.GetAddressOf());
}

bool D3D11Renderer::ConvertFrame(const uint8_t* nv12Data, int width, int height, uint8_t* rgbaOutput) {
    if (!CreateTextures(width, height)) {
        return false;
    }
    
    if (!UpdateNV12Textures(nv12Data, width, height)) {
        return false;
    }
    
    // 更新常量缓冲区
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_context->Map(m_constantBuffer.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (FAILED(hr)) return false;
    
    ConstantBuffer* cb = static_cast<ConstantBuffer*>(mappedResource.pData);
    cb->textureWidth = static_cast<float>(width);
    cb->textureHeight = static_cast<float>(height);
    
    m_context->Unmap(m_constantBuffer.Get(), 0);
    
    // 设置渲染状态
    SetupRenderState();
    
    // 设置渲染目标
    m_context->OMSetRenderTargets(1, m_offscreenRTV.GetAddressOf(), nullptr);
    
    // 设置纹理和常量缓冲区
    ID3D11ShaderResourceView* srvs[] = { m_ySRV.Get(), m_uvSRV.Get() };
    m_context->PSSetShaderResources(0, 2, srvs);
    m_context->PSSetConstantBuffers(0, 1, m_constantBuffer.GetAddressOf());
    
    // 渲染
    m_context->DrawIndexed(6, 0, 0);
    
    // 复制到暂存纹理以便CPU读取
    m_context->CopyResource(m_stagingTexture.Get(), m_offscreenTexture.Get());
    
    // 读取结果
    hr = m_context->Map(m_stagingTexture.Get(), 0, D3D11_MAP_READ, 0, &mappedResource);
    if (FAILED(hr)) return false;
    
    const uint8_t* src = static_cast<const uint8_t*>(mappedResource.pData);
    for (int y = 0; y < height; ++y) {
        memcpy(rgbaOutput + y * width * 4, src + y * mappedResource.RowPitch, width * 4);
    }
    
    m_context->Unmap(m_stagingTexture.Get(), 0);
    
    return true;
}

void D3D11Renderer::RenderToScreen(const uint8_t* nv12Data, int width, int height) {
    if (!nv12Data || !m_context || !m_renderTargetView) {
        static int errorCount = 0;
        if (errorCount < 3) {
            std::cout << "D3D11 RenderToScreen error: nv12Data=" << (nv12Data ? "valid" : "null") 
                      << ", context=" << (m_context ? "valid" : "null")
                      << ", renderTargetView=" << (m_renderTargetView ? "valid" : "null") << std::endl;
            errorCount++;
        }
        return;
    }
    
    static int renderCallCount = 0;
    renderCallCount++;
    if (renderCallCount <= 3) {
        std::cout << "D3D11 RenderToScreen " << renderCallCount << ": " << width << "x" << height << std::endl;
    }
    
    // 创建或更新纹理
    if (!CreateTextures(width, height)) {
        if (renderCallCount <= 3) {
            std::cout << "D3D11 CreateTextures failed" << std::endl;
        }
        return;
    }
    
    // 更新Y纹理数据
    D3D11_MAPPED_SUBRESOURCE mappedResource;
    HRESULT hr = m_context->Map(m_yTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        const uint8_t* srcY = nv12Data;
        uint8_t* dstY = static_cast<uint8_t*>(mappedResource.pData);
        for (int y = 0; y < height; ++y) {
            memcpy(dstY + y * mappedResource.RowPitch, srcY + y * width, width);
        }
        m_context->Unmap(m_yTexture.Get(), 0);
    }
    
    // 更新UV纹理数据
    hr = m_context->Map(m_uvTexture.Get(), 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
    if (SUCCEEDED(hr)) {
        const uint8_t* srcUV = nv12Data + width * height;
        uint8_t* dstUV = static_cast<uint8_t*>(mappedResource.pData);
        int uvWidth = width / 2;
        int uvHeight = height / 2;
        for (int y = 0; y < uvHeight; ++y) {
            memcpy(dstUV + y * mappedResource.RowPitch, srcUV + y * width, width);
        }
        m_context->Unmap(m_uvTexture.Get(), 0);
    }
    
    // 清除渲染目标
    float clearColor[4] = { 0.0f, 0.0f, 0.0f, 1.0f };
    m_context->ClearRenderTargetView(m_renderTargetView.Get(), clearColor);
    
    // 设置渲染目标
    m_context->OMSetRenderTargets(1, m_renderTargetView.GetAddressOf(), nullptr);
    
    // 设置渲染状态
    SetupRenderState();
    
    // 绑定纹理到shader
    ID3D11ShaderResourceView* srvs[] = { m_ySRV.Get(), m_uvSRV.Get() };
    m_context->PSSetShaderResources(0, 2, srvs);
    
    // 渲染
    m_context->DrawIndexed(6, 0, 0);
    
    // 呈现到屏幕
    if (m_swapChain) {
        m_swapChain->Present(1, 0);  // VSync enabled
    }
    
    if (renderCallCount <= 3) {
        std::cout << "D3D11 RenderToScreen completed successfully" << std::endl;
    }
}
