#include "Device.h"
#include "GLFW/glfw3.h"
#include <cassert>
#include <vector>
#include <iostream>
#include <string>
#include <unordered_set>
#include <set>

#ifdef _WIN32
#include <Windows.h>
#endif

namespace sge {
    static VKAPI_ATTR VkBool32 VKAPI_CALL
    debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                  VkDebugUtilsMessageTypeFlagsEXT messageType,
                  const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData, void* pUserData) {
        std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;

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
        assert(deviceCount != 0 && "failed to find GPUs with Vulkan support!");
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(m_instance, &deviceCount, devices.data());
        std::cout << deviceCount << " devices found:\n";
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            std::cout << " * " << deviceProperties.deviceName << " with vulkan version: "
                << VK_API_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                << VK_API_VERSION_MINOR(deviceProperties.apiVersion) << "."
                << VK_API_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
        }
        for (const auto& device : devices) {
            VkPhysicalDeviceProperties deviceProperties;
            vkGetPhysicalDeviceProperties(device, &deviceProperties);
            uint32_t extensionCount;
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
            std::vector<VkExtensionProperties> availableExtensions(extensionCount);
            vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount,
                                                 availableExtensions.data());
            bool ExtFound = false;
            for (auto& reqExt : m_deviceExtensions) {
                for (auto& ext : availableExtensions)
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
                std::cout << "Device used:\n" << " * " << deviceProperties.deviceName << " with vulkan version: "
                    << VK_API_VERSION_MAJOR(deviceProperties.apiVersion) << "."
                    << VK_API_VERSION_MINOR(deviceProperties.apiVersion) << "."
                    << VK_API_VERSION_PATCH(deviceProperties.apiVersion) << std::endl;
                m_physicalDevice = device;
                break;
            }
        }   
        if (m_physicalDevice == VK_NULL_HANDLE) {
            std::cout << "Failed to find suitable GPU!!!\n";
            assert(false && "Failed to find suitable GPU");
        }
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
                std::cout << "Used validation level: " << layer << std::endl;
        } else {
            std::cout << "Vulkan started without validation levels\n";
            createInfo.enabledLayerCount = 0;
        }
        auto result = vkCreateDevice(m_physicalDevice, &createInfo, nullptr, &m_device);
        volkLoadDevice(m_device);   //only for one device!!!!!
        assert(result == VK_SUCCESS && "failed to create logical device!");
        vkGetDeviceQueue(m_device, indices.graphicsFamily, 0, &m_graphicsQueue);
        vkGetDeviceQueue(m_device, indices.presentFamily, 0, &m_presentQueue);
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
        std::cout << "Supported vulkan version: " 
            << VK_API_VERSION_MAJOR(impl_support) << "." 
            << VK_API_VERSION_MINOR(impl_support) << "." 
            << VK_API_VERSION_PATCH(impl_support) << std::endl;

        if (m_enableValidationLayers && !checkValidationLayerSupport())
        {
            std::cout << "Validation layers requested, but not available!\nVulkan api will not use any validation layers!\n\n";
            m_enableValidationLayers = false;
        }

        VkApplicationInfo appInfo{};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "My vulkan app";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "Simplest engine";
        appInfo.pNext = nullptr;
        appInfo.apiVersion = VK_MAKE_VERSION(1, 2, 194);

        std::cout << "Requested vulkan version: "
            << VK_API_VERSION_MAJOR(appInfo.apiVersion) << "."
            << VK_API_VERSION_MINOR(appInfo.apiVersion) << "."
            << VK_API_VERSION_PATCH(appInfo.apiVersion) << std::endl;
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
        assert(result == VK_SUCCESS && "failed to create instance");
        volkLoadInstance(m_instance);
    }

    std::vector<const char*> Device::getRequiredExtentions() {
        uint32_t glfwExtentionCount;
        const char** glfwExtentions; 
        glfwExtentions = glfwGetRequiredInstanceExtensions(&glfwExtentionCount);
        assert(glfwExtentions != nullptr && "Can't get required instance extentions from glfw");
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
        std::cout << "Available extenstions:\n";
        for (const auto& extention : extensions)
        {
            std::cout << "\t" << extention.extensionName << "\n";
            availableExtentions.insert(extention.extensionName);
        }
        std::cout << "Required extentions\n";
        auto requiredExtentions = getRequiredExtentions();
        for (const auto& extention : requiredExtentions) {
            std::cout << "\t" << extention << "\n";
            if (availableExtentions.find(extention) == availableExtentions.end()) 
                missingExtentions.emplace_back(extention);
        }
        for (const auto& extention : missingExtentions) {
            std::cout << "Missing extention: " << extention << "\n";
        }
        assert(missingExtentions.size() == 0 && "Missing required extention!");
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

    void Device::createCommandPool() {
        QueueFamilyIndices queueFamilyIndices = findPhysicalQueueFamilies();

        VkCommandPoolCreateInfo poolInfo = {};
        poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolInfo.queueFamilyIndex = queueFamilyIndices.graphicsFamily;
        poolInfo.flags =
            VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;

        auto result = vkCreateCommandPool(m_device, &poolInfo, nullptr, &m_commandPool);
        assert(result == VK_SUCCESS && "failed to create command pool!");
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
        assert(result == VK_SUCCESS && "Failed to create vertex buffer");

        VkMemoryRequirements memRequirements;
        vkGetBufferMemoryRequirements(m_device, buffer, &memRequirements);
        
        VkMemoryAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
        allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties);
        result = vkAllocateMemory(m_device, &allocInfo, nullptr, &bufferMemory);
        assert(result == VK_SUCCESS && "Failed to allocate vertex buffer memory");
        vkBindBufferMemory(m_device, buffer, bufferMemory, 0);
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
        assert(createResult == VK_SUCCESS && "failed to set up debug messenger!");
    }
}  // namespace sge