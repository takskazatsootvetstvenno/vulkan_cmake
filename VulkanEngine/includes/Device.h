#pragma once
#include "Window.h"

#include <vulkan/vulkan.h>

#include <vector>
namespace sge {
struct QueueFamilyIndices {
    uint32_t graphicsFamily;
    uint32_t presentFamily;
    bool graphicsFamilyHasValue = false;
    bool presentFamilyHasValue = false;
    bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
};

struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities{};
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

class Device {
 public:
    Device(Window& window);
    ~Device();
    Device(const Device&) = delete;
    Device& operator=(const Device&) = delete;
    Device(Device&&) = delete;
    Device& operator=(Device&&) = delete;
    const VkPhysicalDeviceProperties& getPhysicalDeviceProperties() const noexcept;
    SwapChainSupportDetails getSwapChainSupport();
    QueueFamilyIndices findPhysicalQueueFamilies();
    VkDevice device() const noexcept;
    VkPhysicalDevice getPhysicalDevice() const noexcept { return m_physicalDevice; }
    VkSurfaceKHR surface() const noexcept;
    VkQueue graphicsQueue() const noexcept;
    VkQueue presentQueue() const noexcept;
    VkCommandPool getCommandPool() const noexcept;
    VkInstance getInstance() const noexcept;
    bool enableValidationLayers() const noexcept;
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling,
                                 VkFormatFeatureFlags features);
    [[nodiscard]] VkImageView createImageView(const VkImage image, const VkFormat format,
                                              bool isCubeMap = false) noexcept;
    [[nodiscard]] VkSampler createTextureSampler(const VkSamplerCreateInfo& sampleInfo) const noexcept;
    void createImageWithInfo(const VkImageCreateInfo& imageInfo, VkMemoryPropertyFlags properties, VkImage& image,
                             VkDeviceMemory& imageMemory);
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer,
                      VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const;
    void copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height,
                           uint32_t layerCount = 1) const;
    void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout,
                               uint32_t layerCount = 1) const;
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);

    void endSingleTimeCommands(const VkCommandBuffer commandBuffer) const;
    VkCommandBuffer beginSingleTimeCommands() const;

 private:
    void createInstance();
    void setupDebugMessenger();
    void createSurface();
    void pickPhysicalDevice();
    void createLogicalDevice();
    void checkForSupportedExtentions();
    bool checkValidationLayerSupport();
    void createCommandPool();

    std::vector<const char*> getRequiredExtentions();
    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
    SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
    VkPhysicalDeviceProperties m_physicalProperties;
    Window& m_window;
    VkInstance m_instance;
    VkSurfaceKHR m_surface;
    VkDevice m_device;
    VkPhysicalDevice m_physicalDevice = VK_NULL_HANDLE;
    VkQueue m_graphicsQueue;
    VkQueue m_presentQueue;
    VkCommandPool m_commandPool;
    VkDebugUtilsMessengerEXT m_debugMessenger;
    bool m_enableValidationLayers = true;
    const std::vector<const char*> m_validationLayers = {"VK_LAYER_KHRONOS_validation"};
    const std::vector<const char*> m_validationInstanceExtensions = {VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
    const std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME};  //"VK_KHR_synchronization2"
    std::vector<VkExtensionProperties> m_availableExtensions;
};

}  // namespace sge
