#include "Renderer.h"

#include "GLFW/glfw3.h"
#include "Logger.h"
#include "VulkanHelpUtils.h"
#include "ResourceSystem.h"

#include <array>
#include <cassert>
#include <string>
namespace sge {

Renderer::Renderer(Window& window, Device& device) : m_window(window), m_device(device) {
    recreateSwapChain();
    createCommandBuffers();
}

Renderer::~Renderer() { freeCommandBuffers(); }

void Renderer::freeCommandBuffers() {
    vkFreeCommandBuffers(m_device.device(), m_device.getCommandPool(), static_cast<uint32_t>(m_commandBuffers.size()),
                         m_commandBuffers.data());
    m_commandBuffers.clear();
}

void Renderer::recreateSwapChain() noexcept {
    auto extent = m_window.getExtent();
    while (extent.width == 0 || extent.height == 0) {
        extent = m_window.getExtent();
        glfwWaitEvents();
    }
    VK_CHECK_RESULT(vkDeviceWaitIdle(m_device.device()), "Failed to device wait idle while recreating swapchain");
    m_swapChain = nullptr;
    m_swapChain = std::make_unique<SwapChain>(m_device, extent);
    LOG_MSG("New SwapChain has been created!");
}

void Renderer::createCommandBuffers() {
    m_commandBuffers.resize(m_swapChain->imageCount());
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = m_device.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
    auto result = vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data());
    VK_CHECK_RESULT(result, "Failed to allocate command buffers!")
    if (m_device.isEnableValidationLayers()) {
        for (size_t i = 0; i < m_commandBuffers.size(); ++i) {
            std::string temp = "CB_" + std::to_string(i);
            m_device.setObjectName(VK_OBJECT_TYPE_COMMAND_BUFFER, reinterpret_cast<uint64_t>(m_commandBuffers[i]), temp.data());
        }
    }
}

VkCommandBuffer Renderer::beginFrame() noexcept {
    assert(!m_isFrameStarted && "Can't call beginFrame while already in progress");

    auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        recreateSwapChain();
        return nullptr;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        LOG_ERROR("Failed to acquire swap chain image!")
        assert(false);
    }
    m_isFrameStarted = true;
    auto commandBuffer = getCurrentCommandBuffer();
    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    VK_CHECK_RESULT(result, "Failed to begin recording command buffer!")
    return commandBuffer;
}

int Renderer::getFrameIndex() const noexcept {
    assert(m_isFrameStarted && "Cannot get frame index when frame not in progress");
    return m_currentFrameIndex;
}

bool Renderer::endFrame() noexcept {
    bool needCreateNewPipeline = false;
    assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
    auto commandBuffer = getCurrentCommandBuffer();

    VK_CHECK_RESULT(vkEndCommandBuffer(commandBuffer), "Failed to record command buffer!")

    if (auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
        result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.framebufferResized()) {
        m_window.resetWindowResizeFlag();
        recreateSwapChain();
        needCreateNewPipeline = true;
    } else
        VK_CHECK_RESULT(result, "Failed to present swapChain image")
    m_isFrameStarted = false;
    m_currentFrameIndex = (m_currentFrameIndex + 1) % SwapChain::MAX_FRAMES_IN_FLIGHT;
    return needCreateNewPipeline;
}

uint32_t Renderer::getCurrentImageIndex() const noexcept { return m_currentImageIndex; }

void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer frameBuffer) noexcept {
    assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't begin render pass on command buffer from a different frame");

    VkRenderPassBeginInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
   

    /* if (n == 1)
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);
    else
        renderPassInfo.framebuffer = resourceSystem.getFrameBufferByID(n).getFrameBuffer();
        */
    static uint64_t temp = 0;
    if (temp % 3 != 2) {
        renderPassInfo.renderPass = renderPass;
        renderPassInfo.framebuffer = frameBuffer;
    } else {
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);
    }
    ++temp;
    renderPassInfo.renderArea.offset = {0, 0};
    renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
    std::array<VkClearValue, 2> clearValues{};
    clearValues[0].color = {0.1f, 0.1f, 0.1f, 1.f};
    clearValues[1].depthStencil = {1.f, 0};
    renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
    renderPassInfo.pClearValues = clearValues.data();

    vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
}

void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer) noexcept {
    assert(m_isFrameStarted && "Can't call endSwapChainRenderPass if frame is not in progress");
    assert(commandBuffer == getCurrentCommandBuffer() &&
           "Can't end render pass on command buffer from a different frame");
    vkCmdEndRenderPass(commandBuffer);
}

VkRenderPass Renderer::getSwapChainRenderPass() const noexcept { return m_swapChain->getRenderPass(); }

bool Renderer::isFrameInProgress() const { return m_isFrameStarted; }

VkCommandBuffer Renderer::getCurrentCommandBuffer() const noexcept {
    assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress!");
    return m_commandBuffers[m_currentImageIndex];
}
}  // namespace sge
