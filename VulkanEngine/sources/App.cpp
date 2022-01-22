#include "App.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"
#include <array>
namespace sge {
    App::App()
    {
        createPipeLineLayout();
        createPipeline();
        createCommandBuffers();
    }

    App::~App()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void App::run() {
        while (!m_window.shouldClose()) { 
            glfwPollEvents();
            drawFrame();
        }  
        vkDeviceWaitIdle(m_device.device());
    }

    void App::createPipeLineLayout() {
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        auto result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr,
            &m_pipelineLayout);
        assert(result == VK_SUCCESS && "failed to create pipeline layout!");
    }

    void App::createCommandBuffers() {
        m_commandBuffers.resize(m_swapChain.imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());
        auto result = vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data());
        assert(result == VK_SUCCESS && "failed to allocate command buffers!");
        for (int i = 0; i < m_commandBuffers.size(); ++i) {
            VkCommandBufferBeginInfo beginInfo{};
            beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
            result = vkBeginCommandBuffer(m_commandBuffers[i], &beginInfo);
            assert(result == VK_SUCCESS && "failed to begin recording command buffer!");
            VkRenderPassBeginInfo renderPassInfo{};
            renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            renderPassInfo.renderPass = m_swapChain.getRenderPass();
            renderPassInfo.framebuffer = m_swapChain.getFrameBuffer(i);

            renderPassInfo.renderArea.offset = { 0, 0 };
            renderPassInfo.renderArea.extent = m_swapChain.getSwapChainExtent();
            std::array<VkClearValue, 2> clearValues{};
            clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
            clearValues[1].depthStencil = { 1.f, 0 };
            renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
            renderPassInfo.pClearValues = clearValues.data();

            vkCmdBeginRenderPass(m_commandBuffers[i], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
            m_pipeline->bind(m_commandBuffers[i]);
            vkCmdDraw(m_commandBuffers[i], 3, 1, 0, 0);
            vkCmdEndRenderPass(m_commandBuffers[i]);
            auto result = vkEndCommandBuffer(m_commandBuffers[i]);
            assert(result == VK_SUCCESS && "failed to record command buffer");
        }
    }
    void App::drawFrame() {
        uint32_t imageIndex;
        auto result = m_swapChain.acquireNextImage(&imageIndex);
        assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR && "failed to acqure swap chain image!");

        result = m_swapChain.submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
        assert(result == VK_SUCCESS && "failed to present swap chain image");
    }

    void App::createPipeline() {
        auto pipeline_config = Pipeline::createDefaultPipeline(m_swapChain.width(),
            m_swapChain.height());
        pipeline_config.renderPass = m_swapChain.getRenderPass();
        pipeline_config.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<Pipeline>(
            m_device,
            "Shaders/vertex.vert.spv",
            "Shaders/fragment.frag.spv",
            pipeline_config);
    }
}  // namespace sge