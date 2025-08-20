#pragma once

#include <cstdint>

enum class RendererType {
    OpenGL,
    Direct3D11
};

// 抽象渲染器接口
class IRenderer {
public:
    virtual ~IRenderer() = default;
    
    virtual bool Initialize() = 0;
    virtual void Cleanup() = 0;
    
    // NV12到RGBA转换
    virtual bool ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                             uint8_t* rgbaOutput) = 0;
    
    // 直接渲染到屏幕
    virtual void RenderToScreen(const uint8_t* nv12Data, int width, int height) = 0;
    
    virtual RendererType GetType() const = 0;
};

// 渲染器工厂
class RendererFactory {
public:
    static IRenderer* CreateRenderer(RendererType type);
    static void DestroyRenderer(IRenderer* renderer);
};
