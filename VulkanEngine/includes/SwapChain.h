#pragma once
#include "Device.h"

#include <array>

namespace sge {
class SwapChain {
 public:
    static constexpr int MAX_FRAMES_IN_FLIGHT = 2;
    SwapChain(Device& deviceRef, VkExtent2D windowExtent);
    SwapChain(const SwapChain&) = delete;
    SwapChain& operator=(const SwapChain&) = delete;
    ~SwapChain();

    uint32_t width() const noexcept;
    uint32_t height() const noexcept;
    VkFormat getSwapChainImageFormat() const noexcept;
    VkFramebuffer getFrameBuffer(int index) const noexcept;
    VkRenderPass getRenderPass() const noexcept;
    uint32_t imageCount() const noexcept;
    VkExtent2D getSwapChainExtent() const noexcept;
    VkResult acquireNextImage(uint32_t* imageIndex) const noexcept;
    VkResult submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) noexcept;

 private:
    void createSwapChain();
    void createImageViews();
    void createRenderPass();
    void createDepthResources();
    void createFramebuffers();
    void createSyncObjects();

    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);
    VkFormat findDepthFormat();

    Device& m_device;
    VkSwapchainKHR m_swapChain;
    VkRenderPass m_renderPass;
    VkExtent2D m_windowExtent;
    VkExtent2D m_swapChainExtent;
    VkFormat m_swapChainImageFormat;

    std::vector<VkImage> m_swapChainImages;
    std::vector<VkImageView> m_swapChainImageViews;
    std::vector<VkFramebuffer> m_swapChainFramebuffers;

    std::vector<VkImage> m_depthImages;
    std::vector<VkDeviceMemory> m_depthImageMemorys;
    std::vector<VkImageView> m_depthImageViews;

    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_imageAvailableSemaphores;
    std::array<VkSemaphore, MAX_FRAMES_IN_FLIGHT> m_renderFinishedSemaphores;
    std::array<VkFence, MAX_FRAMES_IN_FLIGHT> m_inFlightFences;
    std::vector<VkFence> m_imagesInFlight;
    size_t m_currentFrame = 0;
};
}  // namespace sge
