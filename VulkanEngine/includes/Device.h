#pragma once
#include "volk.h"
#define VK_NO_PROTOTYPES
#include <vector>
#include "Window.h"
namespace sge {
    struct QueueFamilyIndices {
        uint32_t graphicsFamily;
        uint32_t presentFamily;
        bool graphicsFamilyHasValue = false;
        bool presentFamilyHasValue = false;
        bool isComplete() { return graphicsFamilyHasValue && presentFamilyHasValue; }
    };

    struct SwapChainSupportDetails {
        VkSurfaceCapabilitiesKHR capabilities;
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
        SwapChainSupportDetails getSwapChainSupport();
        QueueFamilyIndices findPhysicalQueueFamilies();
        VkDevice device() const noexcept;
        VkSurfaceKHR surface() const noexcept;
        VkQueue graphicsQueue() const noexcept;
        VkQueue presentQueue() const noexcept;
        VkCommandPool getCommandPool() const noexcept;
        VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features);
        void createImageWithInfo(const VkImageCreateInfo& imageInfo,
            VkMemoryPropertyFlags properties,
            VkImage& image,
            VkDeviceMemory& imageMemory);
        void createBuffer(
            VkDeviceSize size,
            VkBufferUsageFlags usage,
            VkMemoryPropertyFlags properties,
            VkBuffer& buffer,
            VkDeviceMemory& bufferMemory);
    private:

        void createInstance();
        void setupDebugMessenger();
        void createSurface();
        void pickPhysicalDevice();
        void createLogicalDevice();
        void checkForSupportedExtentions();
        bool checkValidationLayerSupport();
        void createCommandPool();
        uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties);
        std::vector<const char*> getRequiredExtentions();
        QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device);
        SwapChainSupportDetails querySwapChainSupport(VkPhysicalDevice device);
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
        const std::vector<const char*> m_validationInstanceExtensions = { VK_EXT_DEBUG_UTILS_EXTENSION_NAME};
        const std::vector<const char*> m_deviceExtensions = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, "VK_KHR_synchronization2"};
    };

}  // namespace sge