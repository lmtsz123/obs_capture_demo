#pragma once

#include "renderer_interface.h"

#ifdef VULKAN_AVAILABLE
#define VK_USE_PLATFORM_WIN32_KHR
#include <vulkan/vulkan.h>
#endif

#include <vector>
#include <memory>
#include <string>

#ifdef VULKAN_AVAILABLE
class VulkanRenderer : public IRenderer {
public:
    VulkanRenderer();
    ~VulkanRenderer() override;

    bool Initialize() override;
    bool Initialize(void* windowHandle);  // 接收SDL2窗口句柄
    void Cleanup() override;
    
    bool ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                     uint8_t* rgbaOutput) override;
    
    void RenderToScreen(const uint8_t* nv12Data, int width, int height) override;
    
    RendererType GetType() const override { return RendererType::Vulkan; }

private:
    // Vulkan核心对象
    VkInstance m_instance = VK_NULL_HANDLE;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkDevice m_device = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue = VK_NULL_HANDLE;
    VkQueue m_presentQueue = VK_NULL_HANDLE;
    
    // 交换链相关
    VkSurfaceKHR m_surface = VK_NULL_HANDLE;
    VkSwapchainKHR m_swapChain = VK_NULL_HANDLE;
    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    VkFormat m_swapChainImageFormat;
    VkExtent2D m_swapChainExtent;
    
    // 渲染管线
    VkRenderPass m_renderPass = VK_NULL_HANDLE;
    VkPipelineLayout m_pipelineLayout = VK_NULL_HANDLE;
    VkPipeline m_graphicsPipeline = VK_NULL_HANDLE;
    
    // 帧缓冲
    std::vector<VkFramebuffer> m_swapChainFramebuffers;
    
    // 命令相关
    VkCommandPool m_commandPool = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> m_commandBuffers;
    
    // 同步对象
    std::vector<VkSemaphore> m_imageAvailableSemaphores;
    std::vector<VkSemaphore> m_renderFinishedSemaphores;
    std::vector<VkFence> m_inFlightFences;
    
    // 纹理相关
    VkImage m_yTexture = VK_NULL_HANDLE;
    VkDeviceMemory m_yTextureMemory = VK_NULL_HANDLE;
    VkImageView m_yTextureView = VK_NULL_HANDLE;
    VkSampler m_yTextureSampler = VK_NULL_HANDLE;
    
    VkImage m_uvTexture = VK_NULL_HANDLE;
    VkDeviceMemory m_uvTextureMemory = VK_NULL_HANDLE;
    VkImageView m_uvTextureView = VK_NULL_HANDLE;
    VkSampler m_uvTextureSampler = VK_NULL_HANDLE;
    
    // 描述符相关
    VkDescriptorSetLayout m_descriptorSetLayout = VK_NULL_HANDLE;
    VkDescriptorPool m_descriptorPool = VK_NULL_HANDLE;
    std::vector<VkDescriptorSet> m_descriptorSets;
    
    // 顶点缓冲
    VkBuffer m_vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_vertexBufferMemory = VK_NULL_HANDLE;
    VkBuffer m_indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory m_indexBufferMemory = VK_NULL_HANDLE;
    
    // 队列族索引
    uint32_t m_graphicsFamily = UINT32_MAX;
    uint32_t m_presentFamily = UINT32_MAX;
    
    // 当前帧索引
    size_t m_currentFrame = 0;
    static const int MAX_FRAMES_IN_FLIGHT = 2;
    
    // 窗口句柄
    void* m_windowHandle = nullptr;
    
    // 初始化方法
    bool CreateInstance();
    bool CreateSurface();
    bool PickPhysicalDevice();
    bool CreateLogicalDevice();
    bool CreateSwapChain();
    bool CreateImageViews();
    bool CreateRenderPass();
    bool CreateDescriptorSetLayout();
    bool CreateGraphicsPipeline();
    bool CreateFramebuffers();
    bool CreateCommandPool();
    bool CreateVertexBuffer();
    bool CreateIndexBuffer();
    bool CreateTextures(int width, int height);
    bool CreateDescriptorPool();
    bool CreateDescriptorSets();
    bool CreateCommandBuffers();
    bool CreateSyncObjects();
    
    // 辅助方法
    bool IsDeviceSuitable(VkPhysicalDevice device);
    bool CheckDeviceExtensionSupport(VkPhysicalDevice device);
    void FindQueueFamilies(VkPhysicalDevice device);
    VkShaderModule CreateShaderModule(const std::vector<char>& code);
    uint32_t FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
    void UpdateTextures(const uint8_t* nv12Data, int width, int height);
    std::vector<char> ReadFile(const std::string& filename);
    void CreateBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void CopyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    void RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
    void CreateImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory);
    VkImageView CreateImageView(VkImage image, VkFormat format);
    void TransitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);
    void CopyBufferToImage(VkBuffer buffer, VkImage image, uint32_t width, uint32_t height);
    VkCommandBuffer BeginSingleTimeCommands();
    void EndSingleTimeCommands(VkCommandBuffer commandBuffer);
    
    // 清理方法
    void CleanupSwapChain();
};
#else
// Vulkan不可用时的占位符类
class VulkanRenderer : public IRenderer {
public:
    VulkanRenderer() = default;
    ~VulkanRenderer() override = default;
    
    bool Initialize() override { return false; }
    bool Initialize(void* windowHandle) { return false; }
    void Cleanup() override {}
    
    bool ConvertFrame(const uint8_t* nv12Data, int width, int height, 
                     uint8_t* rgbaOutput) override { return false; }
    
    void RenderToScreen(const uint8_t* nv12Data, int width, int height) override {}
    
    RendererType GetType() const override { return RendererType::Vulkan; }
};
#endif
