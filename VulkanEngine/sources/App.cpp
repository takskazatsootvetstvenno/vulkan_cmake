#include <iostream>
#include <array>

#include "App.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"
namespace sge {
    App::App()
    {
        loadModels();
        createPipeLineLayout();
        recreateSwapChain();
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
    void App::recreateSwapChain() noexcept
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
        std::cout << "New SwapChain has created!\n";
        createPipeline();
    }

    void App::createCommandBuffers() {
        m_commandBuffers.resize(m_swapChain->imageCount());
        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = m_device.getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(m_commandBuffers.size());

        auto result = vkAllocateCommandBuffers(m_device.device(), &allocInfo, m_commandBuffers.data());
        assert(result == VK_SUCCESS && "failed to allocate command buffers!");
        for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
            recordCommandBuffer(i);
    }
    void App::recordCommandBuffer(const int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        auto result = vkBeginCommandBuffer(m_commandBuffers[imageIndex], &beginInfo);
        assert(result == VK_SUCCESS && "failed to begin recording command buffer!");
        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = m_swapChain->getRenderPass();
        renderPassInfo.framebuffer = m_swapChain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = { 0, 0 };
        renderPassInfo.renderArea.extent = m_swapChain->getSwapChainExtent();
        std::array<VkClearValue, 2> clearValues{};
        clearValues[0].color = { 0.1f, 0.1f, 0.1f, 1.f };
        clearValues[1].depthStencil = { 1.f, 0 };
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(m_commandBuffers[imageIndex], &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        m_pipeline->bind(m_commandBuffers[imageIndex]);
        m_model->bind(m_commandBuffers[imageIndex]);
        m_model->draw(m_commandBuffers[imageIndex]);
        vkCmdEndRenderPass(m_commandBuffers[imageIndex]);
        result = vkEndCommandBuffer(m_commandBuffers[imageIndex]);
        assert(result == VK_SUCCESS && "failed to record command buffer");
    }
    void App::drawFrame() noexcept{
        uint32_t imageIndex;
        auto result = m_swapChain->acquireNextImage(&imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR) {
            recreateSwapChain();
            for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
                recordCommandBuffer(i);
            return;
        }
        assert(result == VK_SUCCESS || result == VK_SUBOPTIMAL_KHR && "failed to acquire swap chain image!");
        
        result = m_swapChain->submitCommandBuffers(&m_commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || m_window.framebufferResized())
        {
            m_window.resetWindowResizeFlag();
            recreateSwapChain();
            for (uint32_t i = 0; i < m_commandBuffers.size(); ++i)
                recordCommandBuffer(i);
            return;
        }
        assert(result == VK_SUCCESS && "failed to present swap chain image");
    }

    void App::loadModels()
    {
        std::vector<Model::Vertex> vertices
        {
            {glm::vec2( -0.5f,   0.5f)},
            {glm::vec2(  0.5f,   0.5f)},
            {glm::vec2(  0.0f,  -0.5f)}
        };
        m_model = std::make_unique<Model>(m_device, vertices);
    }

    void App::createPipeline() {
        auto pipeline_config = Pipeline::createDefaultPipeline(m_swapChain->width(),
            m_swapChain->height());
        pipeline_config.renderPass = m_swapChain->getRenderPass();
        pipeline_config.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<Pipeline>(
            m_device,
            "Shaders/vertex.vert.spv",
            "Shaders/fragment.frag.spv",
            pipeline_config);
        std::cout << "New Pipeline has created!\n";
    }
}  // namespace sge