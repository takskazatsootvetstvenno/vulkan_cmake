#include "Renderer.h"
#include <cassert>
#include <array>

#include "Renderer.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"
#include "Logger.h"
namespace sge {
    struct SimplePushConstantData {
        alignas(16) glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };
    Renderer::Renderer(Window& window, Device& device)
        :m_window(window), m_device(device)
    {
        recreateSwapChain();
        createCommandBuffers();
    }

    Renderer::~Renderer()
    {
        freeCommandBuffers();
    }

    void Renderer::freeCommandBuffers() {
        vkFreeCommandBuffers(
            m_device.device(),
            m_device.getCommandPool(),
            static_cast<uint32_t>(m_commandBuffers.size()),
            m_commandBuffers.data());
        m_commandBuffers.clear();
    }

    void Renderer::recreateSwapChain() noexcept
    {
        auto extent = m_window.getExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = m_window.getExtent();
            glfwWaitEvents();
        }
        vkDeviceWaitIdle(m_device.device());
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
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to allocate command buffers!")
            assert(false);
        }
       // for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
       //     recordCommandBuffer(i);
    }

    VkCommandBuffer Renderer::beginFrame()
    {
        assert(!m_isFrameStarted && "Can't call beginFrame while already in progress");

        auto result = m_swapChain->acquireNextImage(&m_currentImageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
           /* for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
                recordCommandBuffer(i);*/
            return nullptr;
        }
        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            LOG_ERROR("Failed to acquire swap chain image!")
            assert(false);
        }
        m_isFrameStarted = true;
        auto commandBuffer = getCurrentCommandBuffer();
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        result = vkBeginCommandBuffer(commandBuffer, &beginInfo);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("failed to begin recording command buffer!")
            assert(false);
        }
        return commandBuffer;
    }
    bool Renderer::endFrame()
    {
        bool needCreateNewPipeline = false;
        assert(m_isFrameStarted && "Can't call endFrame while frame is not in progress");
        auto commandBuffer = getCurrentCommandBuffer();
        
        if (auto result = vkEndCommandBuffer(commandBuffer); result != VK_SUCCESS)
        {
            LOG_ERROR("Failed to record command buffer")
            assert(false);
        }
        if(auto result = m_swapChain->submitCommandBuffers(&commandBuffer, &m_currentImageIndex);
            result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.framebufferResized()) 
        {
            m_window.resetWindowResizeFlag();
            recreateSwapChain();
            needCreateNewPipeline = true;
        }
        else if(result != VK_SUCCESS)
        {
            LOG_ERROR("failed to present swapChain image!")
            assert(false);
        }
        m_isFrameStarted = false;
        return needCreateNewPipeline;
    }
    void Renderer::beginSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_isFrameStarted && "Can't call beginSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't begin render pass on command buffer from a different frame");

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(m_currentImageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
        clearValues[1].depthStencil = { 1.f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);

    }
    void Renderer::endSwapChainRenderPass(VkCommandBuffer commandBuffer)
    {
        assert(m_isFrameStarted && "can't call endSwapChainRenderPass if frame is not in progress");
        assert(commandBuffer == getCurrentCommandBuffer() && "Can't end render pass on command buffer from a different frame");
        vkCmdEndRenderPass(commandBuffer);
    }

	VkRenderPass Renderer::getSwapChainRenderPass() const
	{
		return m_swapChain->getRenderPass();
	}
	bool Renderer::isFrameInProgress() const
	{
		return m_isFrameStarted;
	}
	VkCommandBuffer Renderer::getCurrentCommandBuffer() const
	{
		assert(m_isFrameStarted && "Cannot get command buffer when frame not in progress!");
		return m_commandBuffers[m_currentImageIndex];
	}
}