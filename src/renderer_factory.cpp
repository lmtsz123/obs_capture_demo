#include "renderer_interface.h"
#include "nv12_to_rgba_shader.h"
#include "d3d11_renderer.h"
#include "vulkan_renderer.h"

IRenderer* RendererFactory::CreateRenderer(RendererType type) {
    switch (type) {
        case RendererType::OpenGL:
            return new OpenGLRenderer();
        case RendererType::Direct3D11:
            return new D3D11Renderer();
        case RendererType::Vulkan:
            return new VulkanRenderer();
        default:
            return nullptr;
    }
}

void RendererFactory::DestroyRenderer(IRenderer* renderer) {
    delete renderer;
}
