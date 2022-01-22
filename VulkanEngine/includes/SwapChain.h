#pragma once
#include <vulkan/vulkan.h>
#include "Device.h"
namespace sge {
    class SwapChain {
    public:
        static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
        SwapChain(Device& deviceRef, VkExtent2D windowExtent);
        ~SwapChain();

        SwapChain(const SwapChain&) = delete;
        void operator=(const SwapChain&) = delete;
        uint32_t width() { return m_swapChainExtent.width; }
        uint32_t height() { return m_swapChainExtent.height; }
        VkFormat getSwapChainImageFormat() { return m_swapChainImageFormat; }
        VkFramebuffer getFrameBuffer(int index) { return m_swapChainFramebuffers[index]; }
        VkRenderPass getRenderPass() { return m_renderPass; }
        uint32_t imageCount() { return static_cast<uint32_t>(m_swapChainImages.size()); }
        VkExtent2D getSwapChainExtent() { return m_swapChainExtent; }
        VkResult acquireNextImage(uint32_t* imageIndex);
        VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex);
    private:
        void createSwapChain();
        void createImageViews();
        void createDepthResources();
        VkFormat findDepthFormat();
       
        void createRenderPass();
        void createFramebuffers();

        VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
        VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
        void createSyncObjects();
        VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
        Device& m_device;
        VkExtent2D m_windowExtent;
        VkSwapchainKHR m_swapChain;
        std::vector<VkImage> m_swapChainImages;
        std::vector<VkImageView> m_swapChainImageViews;
        std::vector<VkFramebuffer> m_swapChainFramebuffers;

        std::vector<VkImage> m_depthImages;
        std::vector<VkDeviceMemory> m_depthImageMemorys;
        std::vector<VkImageView> m_depthImageViews;

        VkRenderPass m_renderPass;
        VkFormat m_swapChainImageFormat;
        VkExtent2D m_swapChainExtent;

        std::vector<VkSemaphore> imageAvailableSemaphores;
        std::vector<VkSemaphore> renderFinishedSemaphores;
        std::vector<VkFence> inFlightFences;
        std::vector<VkFence> imagesInFlight;
        size_t m_currentFrame = 0;
    };
}  // namespace sge
