#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>
#include <array>
#include "App.h"
#include "Logger.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"
namespace sge {
    struct SimplePushConstantData {
        alignas(16) glm::vec2 offset;
        alignas(16) glm::vec3 color;
    };
    App::App()
    {
        loadModels();
        createPipeLineLayout();
        createPipeline();
    }

    App::~App()
    {
        vkDestroyPipelineLayout(m_device.device(), m_pipelineLayout, nullptr);
    }

    void App::renderGameObjects(VkCommandBuffer commandBuffer)
    {
        m_pipeline->bind(commandBuffer);

        SimplePushConstantData push{
            glm::vec2(0.3f,0.3f),
            glm::vec3(0.7f, 0.2f, 0.7f) };
        vkCmdPushConstants(
            commandBuffer,
            m_pipelineLayout,
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
            0,
            sizeof(SimplePushConstantData),
            &push);

        m_model->bind(commandBuffer);
        m_model->draw(commandBuffer);
    }
    void App::run() {
        while (!m_window.shouldClose()) {
            glfwPollEvents();
            if (auto commandBuffer = m_renderer.beginFrame())
            {
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                renderGameObjects(commandBuffer);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                if (m_renderer.endFrame() == true)
                    createPipeline();
            }
        }
        vkDeviceWaitIdle(m_device.device());
    }

    void App::createPipeLineLayout() {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(SimplePushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;
        auto result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr,
            &m_pipelineLayout);
        assert(result == VK_SUCCESS && "failed to create pipeline layout!");
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
        assert(m_pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        auto pipeline_config = Pipeline::createDefaultPipeline(m_window.getExtent().width,
            m_window.getExtent().height);
        pipeline_config.renderPass = m_renderer.getSwapChainRenderPass();
        pipeline_config.pipelineLayout = m_pipelineLayout;
        m_pipeline = std::make_unique<Pipeline>(
            m_device,
            "Shaders/vertex.vert.spv",
            "Shaders/fragment.frag.spv",
            pipeline_config);
        LOG_MSG("New Pipeline has been created!");
    }
}  // namespace sge