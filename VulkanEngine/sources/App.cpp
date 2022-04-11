#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES

#include <glm/glm.hpp>
#include <array>
#include "App.h"
#include "Logger.h"
#include "MeshMGR.h"
#include "glm/glm.hpp"
#include "GLFW/glfw3.h"
#include "Buffer.h"
#include "VulkanHelpUtils.h"
#include <vector>
#include <chrono>
#include <numeric>
#include <iterator>

namespace sge {
    App::App()
    {
        m_camera.setViewCircleCamera(15.f, 5.f);
        m_camera.setPerspectiveProjection(glm::radians(90.f), static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height, 0.1f, 100.f);
        auto& mgr = MeshMGR::Instance();
        mgr.setDescriptorPool(DescriptorPool::Builder(m_device)
            .setMaxSets(1000)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000)
            .build());

        mgr.m_generalMatrixUBO = std::make_unique<Buffer>(
                m_device,
                sizeof(GlobalUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
        mgr.m_generalMatrixUBO->map();

        auto descriptorLayout = DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();

        VkPipelineLayout pipelineLayout;
        createPipeLineLayout(descriptorLayout->getDescriptorSetLayout(), pipelineLayout);
        
        mgr.m_pipelines.emplace_back(
            pipelineLayout
        );
        createPipeline(pipelineLayout, mgr.m_pipelines[0].pipeline);
    }

    App::~App()
    {
        auto& mgr = MeshMGR::Instance();
        for (auto& pipeline : mgr.m_pipelines)
        {
            vkDestroyPipelineLayout(m_device.device(), pipeline.pipelineLayout, nullptr);
        }
        mgr.clearTable();
    }

    void App::renderObjects(VkCommandBuffer commandBuffer) noexcept
    {  
        auto& mgr = MeshMGR::Instance();

        for (auto& mesh : mgr.m_meshes)
        {
            mgr.m_pipelines[mesh.getPipelineId()].pipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                mgr.m_pipelines[mesh.getPipelineId()].pipelineLayout,
                0,
                1,
                &mgr.m_sets[mesh.getDescriptorSetId()].set,
                0,
                nullptr);

            m_model->bind(commandBuffer, mesh);
            m_model->draw(commandBuffer, mesh);
        }    
    }
    void App::keyboardProcess() noexcept //TO DO remove this...
    {
        static auto old_time = std::chrono::steady_clock::now();
        auto new_time = std::chrono::steady_clock::now();
        if (std::chrono::duration_cast<std::chrono::milliseconds>(new_time - old_time).count() > 10) {
            if (m_window.getLastKeyInfo().key_space_pressed) {
                m_camera.loadDefaultCircleCamera();
                m_window.getLastKeyInfo().key_space_pressed = false;
            }
            if (m_window.getLastKeyInfo().key_E_pressed)
                m_camera.changeCirclePos(1.f * 3.141592f / 180.f);
            if (m_window.getLastKeyInfo().key_Q_pressed)
                m_camera.changeCirclePos(-1.f * 3.141592f / 180.f);
            if (m_window.getLastKeyInfo().key_W_pressed)
                m_camera.changeCircleHeight(0.5f);
            if (m_window.getLastKeyInfo().key_S_pressed)
                m_camera.changeCircleHeight(-0.5f);
            if (m_window.getLastKeyInfo().scroll_direction != 0)
            {
                m_camera.changeZoom(m_window.getLastKeyInfo().scroll_direction);
                m_window.getLastKeyInfo().scroll_direction = 0;
                m_window.getLastKeyInfo().scroll = 0;
            }
            m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height, 0.1f, 100.f);
            old_time = new_time;
        }
    }
    void App::run() {
        auto& mgr = MeshMGR::Instance();

        while (!m_window.shouldClose()) {
            glfwPollEvents();
            if (auto commandBuffer = m_renderer.beginFrame())
            {
                //update global variables
                GlobalUbo ubo{};
                ubo.projection = m_camera.getProjection();
                ubo.view = m_camera.getView();
                mgr.m_generalMatrixUBO->writeToBuffer(&ubo);
                mgr.m_generalMatrixUBO->flush();
                
                //update keyboard
                keyboardProcess();

                //render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                renderObjects(commandBuffer);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                if (m_renderer.endFrame() == true)
                    for (auto& pipeline : mgr.m_pipelines)
                    {
                        createPipeline(pipeline.pipelineLayout, pipeline.pipeline);
                    }
            }
        }
        vkDeviceWaitIdle(m_device.device());
    }

    void App::createPipeLineLayout(VkDescriptorSetLayout setLayout, VkPipelineLayout& pipelineLayout) noexcept{
        std::vector<VkDescriptorSetLayout> descriptorSetLayouts{ setLayout };
        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
        pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
        pipelineLayoutInfo.pushConstantRangeCount = 0;
        pipelineLayoutInfo.pPushConstantRanges = nullptr;
        auto result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr,
            &pipelineLayout);
        if (result != VK_SUCCESS)
        {
            LOG_ERROR("failed to create pipeline layout\nError: " << getErrorNameFromEnum(result) << " | " << result)
            assert(false);
        }
        
    }

    void App::loadModels(std::vector<Mesh>&& meshess)
    {
        auto& mgr = MeshMGR::Instance();
        auto& mgr_meshes = mgr.m_meshes;
        mgr_meshes.insert(mgr_meshes.end(),
            std::make_move_iterator(meshess.begin()),
            std::make_move_iterator(meshess.end()));
        for (auto& mesh : mgr_meshes)
        {
            auto uboBuffer = std::make_unique<Buffer>(
                m_device,
                sizeof(PBRUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
                );
            uboBuffer->map();

            auto descriptorLayout = DescriptorSetLayout::Builder(m_device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();

            VkDescriptorSet descriptorSet;
            auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
            auto bufferInfo = uboBuffer->descriptorInfo();
            DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &bufferInfo)
                .build(descriptorSet);

            mgr.m_sets.emplace_back(
                std::move(descriptorLayout),
                std::move(uboBuffer),
                descriptorSet
            );

            mesh.m_pipelineId = 0;
            mesh.m_descriptorSetId = mgr.m_sets.size()-1;

            //update
            PBRUbo ubo{};
            ubo.modelMatrix = mesh.getModelMatrix();
            ubo.lightDirection = glm::vec4(1.f, 0.f, 0.f, 0.f);
            ubo.baseColor = mesh.m_material.m_baseColor;
            ubo.metallic = mesh.m_material.m_metallic;
            ubo.roughness = mesh.m_material.m_roughness;
            auto& buffer = mgr.m_sets[mesh.getDescriptorSetId()].uboBuffer;
            buffer->writeToBuffer(&ubo);
            buffer->flush();
        }
        m_model = std::make_unique<Model>(m_device);
    }

    void App::createPipeline(VkPipelineLayout& pipelineLayout, std::unique_ptr<Pipeline>& pipeline) noexcept{
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        auto pipeline_config = Pipeline::createDefaultPipeline(m_window.getExtent().width,
            m_window.getExtent().height);
        pipeline_config.renderPass = m_renderer.getSwapChainRenderPass();
        pipeline_config.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(
            m_device,
            "Shaders/vertex.vert.spv",
            "Shaders/fragment.frag.spv",
            pipeline_config);
        LOG_MSG("New Pipeline has been created!");
    }
}  // namespace sge