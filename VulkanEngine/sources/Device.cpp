#include "Device.h"
#include "VulkanHelpUtils.h"
#include "GLFW/glfw3.h"
#include <cassert>
#include <vector>
#include <string>
#include <unordered_set>
#include <set>
#include <cstring>
#include <Logger.h>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace sge {
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        LOG_ERROR("validation layer: " << pCallbackData->pMessage);

#ifdef _WIN32
        OutputDebugStringA(pCallbackData->pMessage);
        OutputDebugStringA("\n");
#endif
        return VK_FALSE;
    }

    VkResult CreateDebugUtilsMessengerEXT(VkInstance instance,
                                          const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                          const VkAllocationCallbacks* pAllocator,
                                          VkDebugUtilsMessengerEXT* pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
            return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
            return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
    }

    Device::Device(Window& window) : m_window(window) { 
#ifdef NDEBUG
        m_enableValidationLayers = false;
#endif
        createInstance(); 
        setupDebugMessenger();
        createSurface();
        pickPhysicalDevice();
        createLogicalDevice();
        createCommandPool();
    }

    Device::~Device() { 
        vkDestroyCommandPool(m_device, m_commandPool, nullptr);
        vkDestroyDevice(m_device, nullptr);
        if (m_enableValidationLayers) {
           auto func = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(vkGetInstanceProcAddr(
               m_instance, "vkDestroyDebugUtilsMessengerEXT"));
           if (func != nullptr) { func(m_instance, m_debugMessenger, nullptr); }
        }
        vkDestroySurfaceKHR(m_instance, m_surface, nullptr);
        vkDestroyInstance(m_instance, nullptr);
      
    }

    bool Device::checkValidationLayerSupport() {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
        std::vector<VkLayerProperties> layers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, layers.data());
        for (const auto& layer : m_validationLayers) {
            bool layerIsFound = false;
            for (const auto& availableLayer : layers) { 
                if (strcmp(availableLayer.layerName, layer) == 0) {
                    layerIsFound = true;
                    break;
                }
            }
            if (!layerIsFound) return false;
        }
        return true;
    }
    SwapChainSupportDetails Device::querySwapChainSupport(VkPhysicalDevice device) {
        SwapChainSupportDetails details;
        vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, m_surface, &details.capabilities);

        uint32_t formatCount;
        vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount, nullptr);

        if (formatCount != 0) {
            details.formats.resize(formatCount);
            vkGetPhysicalDeviceSurfaceFormatsKHR(device, m_surface, &formatCount,
                                                 details.formats.data());
        }

        uint32_t presentModeCount;
        vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount, nullptr);

        if (presentModeCount != 0) {
            details.presentModes.resize(presentModeCount);
            vkGetPhysicalDeviceSurfacePresentModesKHR(device, m_surface, &presentModeCount,
                                                      details.presentModes.data());
        }
        return details;
    }
    void Device::pickPhysicalDevice() {
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, nullptr);
        if (deviceCount == 0)
        {
            LOG_ERROR("Failed to find GPUs with Vulkan support!")
            assert(false);
        }

        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
        LOG_MSG(deviceCount << " devices found:")
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            LOG_MSG(" * " << deviceProperties.deviceName << " with vulkan version: "
                << VK_API_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                << VK_API_VERSION_MINOR(deviceProperties.apiVersion) << "."
                << VK_API_VERSION_PATCH(deviceProperties.apiVersion))
        }
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            m_availableExtensions.resize(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                                 m_availableExtensions.data());
            bool ExtFound = false;
            for (auto& reqExt : m_deviceExtensions) {
                for (auto& ext : m_availableExtensions)
                    if (strncmp(ext.extensionName, reqExt, 30) == 0) {
                        ExtFound = true;
                        break;
                    }
                if (ExtFound == false) break;
            }
            if(false == ExtFound) continue;
            bool swapChainAdequate = false;
            SwapChainSupportDetails swapChainSupport = querySwapChainSupport(device);
            swapChainAdequate = !swapChainSupport.formats.empty() &&
                                !swapChainSupport.presentModes.empty();
            VkPhysicalDeviceFeatures supportedFeatures;
            vkGetPhysicalDeviceFeatures(device, &supportedFeatures);
            
            if (QueueFamilyIndices indices = findQueueFamilies(device);
                ExtFound && indices.isComplete() && swapChainAdequate &&
                supportedFeatures.samplerAnisotropy) {
                LOG_MSG("Device used:")
                LOG_MSG(" * " << deviceProperties.deviceName << " with vulkan version: "
                    << VK_API_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                    << VK_API_VERSION_MINOR(deviceProperties.apiVersion) << "."
                    << VK_API_VERSION_PATCH(deviceProperties.apiVersion))
                m_physicalDevice = device;
                break;
            }
        }   
        if (m_physicalDevice == VK_NULL_HANDLE) {
            LOG_ERROR("Failed to find suitable GPU!!!");
            assert(false);
        }
        vkGetPhysicalDeviceProperties(m_physicalDevice, &m_physicalProperties);
    }
    void Device::createSurface() { 
        m_window.createWindowSurface(m_instance, &m_surface); 
    }

    void Device::createLogicalDevice() {
        QueueFamilyIndices indices = findQueueFamilies(m_physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily, indices.presentFamily};//for unique queue family id
        float queuePriority = 1.0f;
        for (uint32_t queueFamily : uniqueQueueFamilies) {
            VkDeviceQueueCreateInfo queueCreateInfo = {};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }
        VkPhysicalDeviceFeatures deviceFeatures = {};
        deviceFeatures.samplerAnisotropy = VK_TRUE;

        VkDeviceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;
        createInfo.enabledExtensionCount = static_cast<uint32_t>(m_deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = m_deviceExtensions.data();

        if (m_enableValidationLayers) {
            createInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            createInfo.ppEnabledLayerNames = m_validationLayers.data();
            for (auto layer : m_validationLayers)
                LOG_MSG("Used validation level: " << layer);
        } else {
            LOG_MSG("Vulkan started without validation levels");
            createInfo.enabledLayerCount = 0;
        }

        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create logical device!\nError: " << getErrorNameFromEnum(result) << " : " << result << "\n")
                LOG_MSG("All available device extensions:\n")
                for (auto& device_extenstion : m_availableExtensions)
                    LOG_MSG(device_extenstion.extensionName << " Version: " << device_extenstion.specVersion)
                LOG_MSG("Requested extensions:\n")
                for (auto extenstion : m_deviceExtensions)
                    LOG_MSG(extenstion)
                LOG_MSG_FLUSH
            assert(false);
        }
        volkLoadDevice(m_device);   //only for one device!!!!!
        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
    }
    
    const VkPhysicalDeviceProperties& Device::getPhysicalDeviceProperties() const noexcept
    {
        return m_physicalProperties;
    }

    SwapChainSupportDetails Device::getSwapChainSupport() {
        return querySwapChainSupport(m_physicalDevice); 
    }

    QueueFamilyIndices Device::findPhysicalQueueFamilies() {
        return findQueueFamilies(m_physicalDevice); 
    }

    VkDevice Device::device() const noexcept {
        return m_device; 
    }

    VkSurfaceKHR Device::surface() const noexcept {
        return m_surface;
    }

    VkQueue Device::graphicsQueue() const noexcept{
        return m_graphicsQueue; 
    }

    VkQueue Device::presentQueue() const noexcept {
        return m_presentQueue;
    }

    VkCommandPool Device::getCommandPool() const noexcept
    {
        return m_commandPool; 
    }

    VkInstance Device::getInstance() const noexcept
    {
        return m_instance;
    }

    bool Device::enableValidationLayers() const noexcept
    {
        return m_enableValidationLayers;
    }

    QueueFamilyIndices Device::findQueueFamilies(VkPhysicalDevice device) {

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
        QueueFamilyIndices indices{};
        uint32_t i = 0;
        for (const auto& queueFamily : queueFamilies) {
            if (queueFamily.queueCount > 0 && queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT) {
                indices.graphicsFamily = i;
                indices.graphicsFamilyHasValue = true;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, m_surface, &presentSupport);
            if (queueFamily.queueCount > 0 && presentSupport) {
                indices.presentFamily = i;
                indices.presentFamilyHasValue = true;
            }
            if (indices.isComplete()) break; 
            i++;
        }
        return indices;
    }

    void Device::createInstance() {
        auto vulk = volkInitialize();
        assert(vulk == VK_SUCCESS && "Can't initialize volk loader!");
        uint32_t impl_support;
        vkEnumerateInstanceVersion(&impl_support);
        LOG_MSG("Supported vulkan version: "
            << VK_API_VERSION_MAJOR(impl_support) << "." 
            << VK_API_VERSION_MINOR(impl_support) << "." 
            << VK_API_VERSION_PATCH(impl_support));

        if (m_enableValidationLayers && !checkValidationLayerSupport())
        {
            LOG_ERROR("Validation layers requested, but not available!\nVulkan api will not use any validation layers!\n");
            m_enableValidationLayers = false;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "My vulkan app";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Simplest engine";
        appInfo.pNext = nullptr;
        appInfo.apiVersion = VK_MAKE_VERSION(1, 3, 194);

        LOG_MSG("Requested vulkan version: "
            << VK_API_VERSION_MAJOR(appInfo.apiVersion) << "."
            << VK_API_VERSION_MINOR(appInfo.apiVersion) << "."
            << VK_API_VERSION_PATCH(appInfo.apiVersion))
        checkForSupportedExtentions();

        VkInstanceCreateInfo instanceCreateInfo{};
        instanceCreateInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        instanceCreateInfo.pApplicationInfo = &appInfo;

        auto requiredExtensions = getRequiredExtentions();

        instanceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
        instanceCreateInfo.ppEnabledExtensionNames = requiredExtensions.data();

        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        
        if (m_enableValidationLayers) { 
            instanceCreateInfo.enabledLayerCount = static_cast<uint32_t>(m_validationLayers.size());
            instanceCreateInfo.ppEnabledLayerNames = m_validationLayers.data();

            debugCreateInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
            debugCreateInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                              VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
            debugCreateInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                          VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
            debugCreateInfo.pfnUserCallback = debugCallback;
            instanceCreateInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
        } else {
            instanceCreateInfo.enabledLayerCount = 0;
            instanceCreateInfo.pNext = nullptr;
        }
        VkResult result = vkCreateInstance(&instanceCreateInfo, nullptr, &m_instance);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create a vulkan instance!\nError: "  << getErrorNameFromEnum(result) << " : " << result);
            assert(false);
        }
        volkLoadInstance(m_instance);
    }

    std::vector<const char*> Device::getRequiredExtentions() {
        uint32_t glfwExtentionCount;
        const char** glfwExtentions; 
        glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);
        if (glfwExtentions == nullptr)
        {
            LOG_ERROR("Can't get required instance extentions from glfw")
            assert(false);
        }
      
        std::vector<const char*> allExtentions(glfwExtentions, glfwExtentions + glfwExtentionCount);

        if (m_enableValidationLayers)
        {
            allExtentions.insert(allExtentions.end(), m_validationInstanceExtensions.begin(), m_validationInstanceExtensions.end());
        }
        return allExtentions;
    }

    void Device::checkForSupportedExtentions() {
        uint32_t extensionCount = 0;
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, nullptr);
        std::vector<VkExtensionProperties> extensions(extensionCount);
        vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount, extensions.data());
        std::unordered_set<std::string> availableExtentions;
        std::vector<std::string> missingExtentions;
        LOG_MSG("Available extenstions:")
        for (const auto& extention : extensions)
        {
            LOG_MSG("\t" << extention.extensionName);
            availableExtentions.insert(extention.extensionName);
        }
        LOG_MSG("Required extentions");
        auto requiredExtentions = getRequiredExtentions();
        for (const auto& extention : requiredExtentions) {
            LOG_MSG("\t" << extention);
            if (availableExtentions.find(extention) == availableExtentions.end()) 
                missingExtentions.emplace_back(extention);
        }
        for (const auto& extention : missingExtentions) {
            LOG_ERROR("Missing extention: " << extention);

        }
        if (!missingExtentions.empty())
        {
            LOG_ERROR("Missing required extention!")
            assert(false);
        }
    }
    VkFormat Device::findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features)
    {
        for (VkFormat format : candidates) {
            VkFormatProperties props;
            vkGetPhysicalDeviceFormatProperties(m_physicalDevice, format, &props);

            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) {
                return format;
            }
            else if (
                tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) {
                return format;
            }
        }
        assert(false && "failed to find supported format!");
    }

    [[nodiscard]] VkImageView Device::createImageView(const VkImage image, const VkFormat format) noexcept
    {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = image;
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = format;
        createInfo.components.r = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.g = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.b = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.components.a = VK_COMPONENT_SWIZZLE_IDENTITY;
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;

        VkImageView imageView;
        auto result = vkCreateImageView(m_device, &createInfo, nullptr, &imageView);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("failed to create image views!\nError: " << getErrorNameFromEnum(result) << " | " << result)
            assert(false);
        }
        return imageView;
    }

    VkSampler Device::createTextureSampler(const VkSamplerCreateInfo& sampleInfo) const noexcept
    {
        VkSampler textureSampler;
        auto result = vkCreateSampler(m_device, &sampleInfo, nullptr, &textureSampler);
        if (result != VK_SUCCESS) {
            LOG_ERROR("Failed to create texture sampler!");
            assert(false);
        }
        return textureSampler;
    }

    void Device::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("failed to create command pool!\nError: " << getErrorNameFromEnum(result) << " : " << result);
            assert(false);
        }
    }

    void Device::endSingleTimeCommands(const VkCommandBuffer commandBuffer) const
    {
        vkEndCommandBuffer(commandBuffer);

        VkSubmitInfo submitInfo{};
        submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
        submitInfo.commandBufferCount = 1;
        submitInfo.pCommandBuffers = &commandBuffer;

        vkQueueSubmit(m_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
        vkQueueWaitIdle(m_graphicsQueue);

        vkFreeCommandBuffers(m_device, m_commandPool, 1, &commandBuffer);
    }

    VkCommandBuffer Device::beginSingleTimeCommands() const
    {
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_commandPool;
        allocInfo.commandBufferCount = 1;

        VkCommandBuffer commandBuffer;
        vkAllocateCommandBuffers(m_device, &allocInfo, &commandBuffer);

        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

        vkBeginCommandBuffer(commandBuffer, &beginInfo);
        return commandBuffer;
    }

    uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) {
        VkPhysicalDeviceMemoryProperties memProperties;
        vkGetPhysicalDeviceMemoryProperties(m_physicalDevice, &memProperties);
        for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) {
            if ((typeFilter & (1 << i)) &&
                (memProperties.memoryTypes[i].propertyFlags & properties) == properties) {
                return i;
            }
        }
        assert(false && "failed to find suitable memory type!");
    }

    void Device::createImageWithInfo(
        const VkImageCreateInfo& imageInfo,
        VkMemoryPropertyFlags properties,
        VkImage& image,
        VkDeviceMemory& imageMemory)
    {
        auto result = vkCreateImage(m_device, &imageInfo, nullptr, &image);
        assert(result == VK_SUCCESS && "failed to create image!");

        VkMemoryRequirements memRequirements;
        vkGetImageMemoryRequirements(m_device, image, &memRequirements);

        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);

        result = vkAllocateMemory(m_device, &allocInfo, nullptr, &imageMemory);
        assert(result == VK_SUCCESS && "failed to allocate image memory!");

        result = vkBindImageMemory(m_device, image, imageMemory, 0);
        assert(result == VK_SUCCESS && "failed to bind image memory!");
    }

    void Device::createBuffer(
        VkDeviceSize size,
        VkBufferUsageFlags usage,
        VkMemoryPropertyFlags properties,
        VkBuffer& buffer,
        VkDeviceMemory& bufferMemory)
    {
        VkBufferCreateInfo bufferInfo{};
        bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
        bufferInfo.size = size;
        bufferInfo.usage = usage;
        bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        auto result = vkCreateBuffer(m_device, &bufferInfo, nullptr, &buffer);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to create vertex buffer!\nError: " << getErrorNameFromEnum(result) << " : " << result);
            assert(false);
        }   
        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to allocate vertex buffer memory!\nError: " << getErrorNameFromEnum(result) << " : " << result);
            assert(false);
        }
        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
    }

    void Device::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) const {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkBufferCopy copyRegion{};
        copyRegion.srcOffset = 0;  // Optional
        copyRegion.dstOffset = 0;  // Optional
        copyRegion.size = size;
        vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion);

        endSingleTimeCommands(commandBuffer);
    }

    void Device::copyBufferToImage(VkBuffer srcBuffer, VkImage dstImage, uint32_t width, uint32_t height) const {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();
        VkBufferImageCopy region{};
        region.bufferOffset = 0;
        region.bufferRowLength = 0;
        region.bufferImageHeight = 0;

        region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        region.imageSubresource.mipLevel = 0;
        region.imageSubresource.baseArrayLayer = 0;
        region.imageSubresource.layerCount = 1;

        region.imageOffset = { 0, 0, 0 };
        region.imageExtent = {
            width,
            height,
            1
        };

        vkCmdCopyBufferToImage(
            commandBuffer,
            srcBuffer,
            dstImage,
            VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
            1,
            &region
        );
        endSingleTimeCommands(commandBuffer);
    }

    void Device::transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout) const {
        VkCommandBuffer commandBuffer = beginSingleTimeCommands();

        VkImageMemoryBarrier barrier{};
        barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
        barrier.oldLayout = oldLayout;
        barrier.newLayout = newLayout;

        barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
        barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;

        barrier.image = image;
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        barrier.subresourceRange.baseMipLevel = 0;
        barrier.subresourceRange.levelCount = 1;
        barrier.subresourceRange.baseArrayLayer = 0;
        barrier.subresourceRange.layerCount = 1;

        VkPipelineStageFlags sourceStage;
        VkPipelineStageFlags destinationStage;

        if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
            barrier.srcAccessMask = 0;
            barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

            sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
            destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
        }
        else if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
            barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
            barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

            sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
            destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
        }
        else {
            LOG_ERROR("unsupported layout transition!");
            assert(false);
        }

        vkCmdPipelineBarrier(
            commandBuffer,
            sourceStage,
            destinationStage,
            0,
            0, nullptr,
            0, nullptr,
            1, &barrier
        );
        endSingleTimeCommands(commandBuffer);
    }

    void Device::setupDebugMessenger() {
        if (!m_enableValidationLayers) return;
        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                                     VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                                 VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
        createInfo.pUserData = nullptr;
        auto createResult = CreateDebugUtilsMessengerEXT(m_instance, &createInfo, nullptr, &m_debugMessenger);
        if (createResult != VK_SUCCESS)
        {
            LOG_ERROR("failed to set up debug messenger!\nError: " << getErrorNameFromEnum(createResult) << " : " << createResult);
            assert(false);
        }
    }
}  // namespace sge
