
#include "SwapChain.h"

#include "Logger.h"
#include "VulkanHelpUtils.h"

#include <algorithm>
#include <array>
#include <cassert>
#include <limits>
namespace sge {
SwapChain::SwapChain(Device& deviceRef, VkExtent2D windowExtent) : m_device(deviceRef), m_windowExtent(windowExtent) {
    createSwapChain();
    createImageViews();
    createRenderPass();
    createDepthResources();
    createFramebuffers();
    createSyncObjects();
}

SwapChain::~SwapChain() {
    for (auto imageView : m_swapChainImageViews) vkDestroyImageView(m_device.device(), imageView, nullptr);
    m_swapChainImageViews.clear();

    if (m_swapChain != nullptr) {
        vkDestroySwapchainKHR(m_device.device(), m_swapChain, nullptr);
        m_swapChain = nullptr;
    }

    for (size_t i = 0; i < m_depthImages.size(); ++i) {
        vkDestroyImageView(m_device.device(), m_depthImageViews[i], nullptr);
        vkDestroyImage(m_device.device(), m_depthImages[i], nullptr);
        vkFreeMemory(m_device.device(), m_depthImageMemorys[i], nullptr);
    }

    for (auto framebuffer : m_swapChainFramebuffers) vkDestroyFramebuffer(m_device.device(), framebuffer, nullptr);

    vkDestroyRenderPass(m_device.device(), m_renderPass, nullptr);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        vkDestroySemaphore(m_device.device(), m_renderFinishedSemaphores[i], nullptr);
        vkDestroySemaphore(m_device.device(), m_imageAvailableSemaphores[i], nullptr);
        vkDestroyFence(m_device.device(), m_inFlightFences[i], nullptr);
    }
}

uint32_t SwapChain::width() const noexcept { return m_swapChainExtent.width; }

uint32_t SwapChain::height() const noexcept { return m_swapChainExtent.height; }

VkFormat SwapChain::getSwapChainImageFormat() const noexcept { return m_swapChainImageFormat; }

VkFramebuffer SwapChain::getFrameBuffer(int index) const noexcept { return m_swapChainFramebuffers[index]; }

VkRenderPass SwapChain::getRenderPass() const noexcept { return m_renderPass; }

uint32_t SwapChain::imageCount() const noexcept { return static_cast<uint32_t>(m_swapChainImages.size()); }

VkExtent2D SwapChain::getSwapChainExtent() const noexcept { return m_swapChainExtent; }

VkSurfaceFormatKHR SwapChain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) {
    for (const auto& availableFormat : availableFormats) {
        if (availableFormat.format == VK_FORMAT_B8G8R8A8_UNORM &&
            availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) {
            return availableFormat;
        }
    }
    return availableFormats[0];
}

VkPresentModeKHR SwapChain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) {
    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) {
            LOG_MSG("Present mode: Mailbox");
            return availablePresentMode;
        }

    for (const auto& availablePresentMode : availablePresentModes)
        if (availablePresentMode == VK_PRESENT_MODE_IMMEDIATE_KHR) {
            LOG_MSG("Present mode: Immediate");
            return availablePresentMode;
        }

    LOG_MSG("Present mode: V-Sync");
    return VK_PRESENT_MODE_FIFO_KHR;
}

void SwapChain::createSyncObjects() {
    m_imagesInFlight.resize(imageCount(), VK_NULL_HANDLE);
    VkSemaphoreCreateInfo semaphoreInfo = {};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo = {};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i) {
        if (vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_imageAvailableSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateSemaphore(m_device.device(), &semaphoreInfo, nullptr, &m_renderFinishedSemaphores[i]) !=
                VK_SUCCESS ||
            vkCreateFence(m_device.device(), &fenceInfo, nullptr, &m_inFlightFences[i]) != VK_SUCCESS) {
            LOG_ERROR("Failed to create synchronization objects for a frame!")
            assert(false);
        }
    }
}

VkExtent2D SwapChain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities) {
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
    VkExtent2D actualExtent = m_windowExtent;
    actualExtent.width = std::max(capabilities.minImageExtent.width,
                                  std::min(capabilities.maxImageExtent.width, actualExtent.width));
    actualExtent.height = std::max(capabilities.minImageExtent.height,
                                   std::min(capabilities.maxImageExtent.height, actualExtent.height));

    return actualExtent;
}

void SwapChain::createSwapChain() {
    SwapChainSupportDetails swapChainSupport = m_device.getSwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities);

    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && 3 <= swapChainSupport.capabilities.maxImageCount) {
        imageCount = 3;
    }
    VkSwapchainCreateInfoKHR createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = m_device.surface();
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    QueueFamilyIndices indices = m_device.findPhysicalQueueFamilies();
    uint32_t queueFamilyIndices[] = {indices.graphicsFamily, indices.presentFamily};

    if (indices.graphicsFamily != indices.presentFamily) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndices;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;
    auto result = vkCreateSwapchainKHR(m_device.device(), &createInfo, nullptr, &m_swapChain);
    VK_CHECK_RESULT(result, "Failed to create swap chain!")
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, nullptr),
                    "Failed to get count of swapchain images");
    m_swapChainImages.resize(imageCount);
    VK_CHECK_RESULT(vkGetSwapchainImagesKHR(m_device.device(), m_swapChain, &imageCount, m_swapChainImages.data()),
                    "Failed to get swapchain images");
    m_swapChainImageFormat = surfaceFormat.format;
    m_swapChainExtent = extent;
}

void SwapChain::createImageViews() {
    m_swapChainImageViews.resize(m_swapChainImages.size());
    for (size_t i = 0; i < m_swapChainImages.size(); ++i)
        m_swapChainImageViews[i] = m_device.createImageView(m_swapChainImages[i], m_swapChainImageFormat);
}

void SwapChain::createDepthResources() {
    VkFormat depthFormat = findDepthFormat();
    VkExtent2D swapChainExtent = m_swapChainExtent;

    m_depthImages.resize(m_swapChainImages.size());
    m_depthImageMemorys.resize(m_swapChainImages.size());
    m_depthImageViews.resize(m_swapChainImages.size());

    for (size_t i = 0; i < m_depthImages.size(); ++i) {
        VkImageCreateInfo imageInfo{};
        imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
        imageInfo.imageType = VK_IMAGE_TYPE_2D;
        imageInfo.extent.width = swapChainExtent.width;
        imageInfo.extent.height = swapChainExtent.height;
        imageInfo.extent.depth = 1;
        imageInfo.mipLevels = 1;
        imageInfo.arrayLayers = 1;
        imageInfo.format = depthFormat;
        imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
        imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
        imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
        imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        imageInfo.flags = 0;

        m_device.createImageWithInfo(imageInfo, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, m_depthImages[i],
                                     m_depthImageMemorys[i]);

        VkImageViewCreateInfo viewInfo{};
        viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        viewInfo.image = m_depthImages[i];
        viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        viewInfo.format = depthFormat;
        viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
        viewInfo.subresourceRange.baseMipLevel = 0;
        viewInfo.subresourceRange.levelCount = 1;
        viewInfo.subresourceRange.baseArrayLayer = 0;
        viewInfo.subresourceRange.layerCount = 1;

        auto result = vkCreateImageView(m_device.device(), &viewInfo, nullptr, &m_depthImageViews[i]);
        VK_CHECK_RESULT(result, "Failed to create texture image view!")
    }
}

VkFormat SwapChain::findDepthFormat() {
    return m_device.findSupportedFormat(
        {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT}, VK_IMAGE_TILING_OPTIMAL,
        VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}

VkResult SwapChain::acquireNextImage(uint32_t* imageIndex) const noexcept {
    vkWaitForFences(  // Ждём, когда ВСЕ ранее записанные команды в командном буффере исполнятся.
        m_device.device(), 1, &m_inFlightFences[m_currentFrame], VK_TRUE, std::numeric_limits<uint64_t>::max());

    VkResult result = vkAcquireNextImageKHR(  // запрос изображения из swapChain для дальнейшего рендера
        m_device.device(), m_swapChain, std::numeric_limits<uint64_t>::max(),
        m_imageAvailableSemaphores[m_currentFrame],  // как только изображение считается из presentEngine, взведётся
                                                     // этот семофор в GPU
        VK_NULL_HANDLE, imageIndex);

    return result;
}

VkResult SwapChain::submitCommandBuffers(const VkCommandBuffer* buffers, uint32_t* imageIndex) noexcept {
    if (m_imagesInFlight[*imageIndex] != VK_NULL_HANDLE) {  // Проверяем, чтобы изображения рендерели по порядку, т.е.
                                                            // чтобы рендерелись в порядке 0 1 2 0 1 2 ...
        vkWaitForFences(m_device.device(), 1, &m_imagesInFlight[*imageIndex], VK_TRUE, UINT64_MAX);
    }
    m_imagesInFlight[*imageIndex] = m_inFlightFences[m_currentFrame];
    VkSubmitInfo submitInfo = {};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {m_imageAvailableSemaphores[m_currentFrame]};
    VkPipelineStageFlags waitStages[] = {
        VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};  // указываем, на какую стадию впихнуть семофор, все стадии до
                                                         // этой - без блокировки в GPU
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = buffers;

    VkSemaphore signalSemaphores[] = {m_renderFinishedSemaphores[m_currentFrame]};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VK_CHECK_RESULT(vkResetFences(m_device.device(), 1, &m_inFlightFences[m_currentFrame]),
                    "Failed to reset fence");  // Сброс fence

    auto result = vkQueueSubmit(
        m_device.graphicsQueue(), 1, &submitInfo,
        m_inFlightFences[m_currentFrame]);  // Отправляем командный буффер, причём в waitStages будет стоять семофор
                                            // ожидающий конца чтения изображения из presentEngine(сигнал конца
                                            // vkAcquireNextImageKHR даёт в виде семофора),
    VK_CHECK_RESULT(result, "Failed to submit draw command buffer!")
    VkPresentInfoKHR presentInfo =
        {};  // а после того как все команды буффера исполнятся будет взведён fence, означающий, что буффер пуст
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;

    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {m_swapChain};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;

    presentInfo.pImageIndices = imageIndex;

    m_currentFrame = (m_currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;

    result = vkQueuePresentKHR(
        m_device.presentQueue(),
        &presentInfo);  // Отправляем изображение в очередь представления, причём очередь представления будет дожидаться
                        // завершение выполнения командного буффера (всех команд, что попали в вызов vkQueueSubmit)
    return result;
}

void SwapChain::createRenderPass() {
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = findDepthFormat();
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1;
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    VkAttachmentDescription colorAttachment = {};
    colorAttachment.format = getSwapChainImageFormat();
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

    VkAttachmentReference colorAttachmentRef = {};
    colorAttachmentRef.attachment = 0;
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    VkSubpassDescription subpass = {};  // layout(location = 0) out vec4 outColor
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef;
    subpass.pDepthStencilAttachment = &depthAttachmentRef;

    VkSubpassDependency dependency = {};
    {  // External --> subpass 0
        dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
        dependency.dstSubpass = 0;
        dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT |
                                  VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
        dependency.srcAccessMask = 0;
        dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;
    }
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency;

    auto result = vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass);
    VK_CHECK_RESULT(result, "Failed to create render pass!")
}

void SwapChain::createFramebuffers() {
    auto image_count = imageCount();
    m_swapChainFramebuffers.resize(image_count);
    for (size_t i = 0; i < image_count; ++i) {
        std::array<VkImageView, 2> attachments = {m_swapChainImageViews[i], m_depthImageViews[i]};

        VkExtent2D swapChainExtent = m_swapChainExtent;
        VkFramebufferCreateInfo framebufferInfo = {};
        framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
        framebufferInfo.renderPass = m_renderPass;
        framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
        framebufferInfo.pAttachments = attachments.data();
        framebufferInfo.width = swapChainExtent.width;
        framebufferInfo.height = swapChainExtent.height;
        framebufferInfo.layers = 1;

        auto result = vkCreateFramebuffer(m_device.device(), &framebufferInfo, nullptr, &m_swapChainFramebuffers[i]);
        VK_CHECK_RESULT(result, "Failed to create framebuffer!")
    }
}
}  // namespace sge
