#pragma once

#include "renderer_interface.h"
#include <d3d11.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <wrl/client.h>

using Microsoft::WRL::ComPtr;

class D3D11Renderer : public IRenderer {
public:
    D3D11Renderer();
    ~D3D11Renderer() override;

    bool Initialize() override;
    void Cleanup() override;
    
    bool ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                     uint8_t* rgbaOutput) override;
    
    void RenderToScreen(const uint8_t* nv12Data, int width, int height) override;
    
    RendererType GetType() const override { return RendererType::Direct3D11; }

private:
    // D3D11 设备和上下文
    ComPtr<ID3D11Device> m_device;
    ComPtr<ID3D11DeviceContext> m_context;
    ComPtr<IDXGISwapChain> m_swapChain;
    ComPtr<ID3D11RenderTargetView> m_renderTargetView;
    
    // Shader相关
    ComPtr<ID3D11VertexShader> m_vertexShader;
    ComPtr<ID3D11PixelShader> m_pixelShader;
    ComPtr<ID3D11InputLayout> m_inputLayout;
    
    // 纹理和资源
    ComPtr<ID3D11Texture2D> m_yTexture;
    ComPtr<ID3D11Texture2D> m_uvTexture;
    ComPtr<ID3D11ShaderResourceView> m_ySRV;
    ComPtr<ID3D11ShaderResourceView> m_uvSRV;
    ComPtr<ID3D11SamplerState> m_samplerState;
    
    // 顶点缓冲区
    ComPtr<ID3D11Buffer> m_vertexBuffer;
    ComPtr<ID3D11Buffer> m_indexBuffer;
    ComPtr<ID3D11Buffer> m_constantBuffer;
    
    // 用于离屏渲染的资源
    ComPtr<ID3D11Texture2D> m_offscreenTexture;
    ComPtr<ID3D11RenderTargetView> m_offscreenRTV;
    ComPtr<ID3D11Texture2D> m_stagingTexture;
    
    struct ConstantBuffer {
        float textureWidth;
        float textureHeight;
        float padding[2];
    };
    
    bool CreateDevice();
    bool CreateShaders();
    bool CreateBuffers();
    bool CreateTextures(int width, int height);
    bool UpdateNV12Textures(const uint8_t* nv12Data, int width, int height);
    void SetupRenderState();
};
