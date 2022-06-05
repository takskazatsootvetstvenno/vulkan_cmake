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
#include "Shader.h"
#include "Texture.h"

namespace sge {
    App::App()
    {
        m_camera.setViewCircleCamera(15.f, 5.f);
        m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height, 0.1f, 100.f);
        initEvents();
        auto& mgr = MeshMGR::Instance();
        mgr.setDescriptorPool(DescriptorPool::Builder(m_device)
            .setMaxSets(1024)
            .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024)
            .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024)
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
            .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        
        if (Shader glslPBRShader("Shaders/GLSL/PBR/PBR.vert", "Shaders/GLSL/PBR/PBR.frag"); glslPBRShader.isValid())
        {
            auto pipelineLayoutGLSLPBR = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
            mgr.m_pipelines.emplace_back(pipelineLayoutGLSLPBR, nullptr);
            createPipeline(pipelineLayoutGLSLPBR, mgr.m_pipelines.back().pipeline, std::move(glslPBRShader));
        }
        if (Shader glslPhongShader("Shaders/GLSL/Phong/phong.vert", "Shaders/GLSL/Phong/phong.frag"); glslPhongShader.isValid())
        {
            auto pipelineLayoutGLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
            mgr.m_pipelines.emplace_back(pipelineLayoutGLSLPhong, nullptr);
            createPipeline(pipelineLayoutGLSLPhong, mgr.m_pipelines.back().pipeline, std::move(glslPhongShader));
        }
        if (Shader hlslPhongShader("Shaders/HLSL/Phong/phong_vert.hlsl", "Shaders/HLSL/Phong/phong_frag.hlsl"); hlslPhongShader.isValid())
        {
            auto pipelineLayoutHLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
            mgr.m_pipelines.emplace_back(pipelineLayoutHLSLPhong, nullptr);
            createPipeline(pipelineLayoutHLSLPhong, mgr.m_pipelines.back().pipeline, std::move(hlslPhongShader));
        }
        if (Shader hlslPBRShader("Shaders/HLSL/PBR/PBR_vert.hlsl", "Shaders/HLSL/PBR/PBR_frag.hlsl"); hlslPBRShader.isValid())
        {
            auto pipelineLayoutHLSLPBR = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
            mgr.m_pipelines.emplace_back(pipelineLayoutHLSLPBR, nullptr);
            createPipeline(pipelineLayoutHLSLPBR, mgr.m_pipelines.back().pipeline, std::move(hlslPBRShader));
        }
        m_model = std::make_unique<Model>(m_device);
    }

    App::~App()
    {
        auto& mgr = MeshMGR::Instance();
        for (auto& pipeline : mgr.m_pipelines)
        {
            vkDestroyPipelineLayout(m_device.device(), pipeline.pipelineLayout, nullptr);
        }
        for (auto& textures : mgr.m_textures)
        {
            vkDestroyImage(m_device.device(), textures.second.getTextureImage(), nullptr);
            vkFreeMemory(m_device.device(), textures.second.getTextureImageMemory(), nullptr);
            vkDestroyImageView(m_device.device(), textures.second.getImageView(), nullptr);
            vkDestroySampler(m_device.device(), textures.second.getSampler(), nullptr);
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

    void App::initEvents() noexcept
    {
        m_eventDispatcher.add_event_listener <EventKeyPressed>(
            [&](EventKeyPressed& event)
            {
                switch (event.key)
                {
                case 69:	//E
                    m_camera.changeCirclePos(5.f * 3.141592f / 180.f);

                    break;
                case 71:	//G
                    if (event.action == 0){
                        auto& mgr = MeshMGR::Instance();
                        auto numOfPipelines = mgr.m_pipelines.size();
                        static int current_pipeline = 0;
                        if (numOfPipelines < 2) break;
                        std::swap(
                            mgr.m_pipelines[current_pipeline % numOfPipelines],
                            mgr.m_pipelines[0]
                        );
                        std::swap(
                            mgr.m_pipelines[0],
                            mgr.m_pipelines[(current_pipeline+1) % numOfPipelines]
                        );
                        (++current_pipeline) %= numOfPipelines;
                        LOG_MSG("Pipeline #" << current_pipeline << ": " << mgr.m_pipelines[0].pipeline->getShader().getFragmentShaderPath());
                     }
                    break;
                case 83:	//S
                    m_camera.changeCircleHeight(-1.f);
                    break;
                case 87:	//W
                    m_camera.changeCircleHeight(1.f);
                    break;
                case 81:	//Q
                    m_camera.changeCirclePos(-5.f * 3.141592f / 180.f);
                    break;
                case 256:	//GLFW_KEY_ESCAPE
                    m_window.closeWindow();
                    break;
                }
            }
        );
        m_eventDispatcher.add_event_listener <EventScroll>(
            [&](EventScroll& event)
            {
                m_camera.changeZoom(static_cast<float>(-event.y * 2));
                m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height, 0.1f, 100.f);
            }
        );
        m_eventDispatcher.add_event_listener <EventWindowResize>(
            [&](EventWindowResize& event)
            {
                m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height, 0.1f, 100.f);
            }
        );
        m_window.set_event_callback(
            [&](BaseEvent& event)
            {
                m_eventDispatcher.dispatch(event);
            }
        );
    }
    void App::run() {
        m_model->createBuffers();
        auto& mgr = MeshMGR::Instance();

        while (!m_window.shouldClose()) {
            glfwPollEvents();
            if (auto commandBuffer = m_renderer.beginFrame())
            {
                //update global variables
                GlobalUbo ubo{};
                ubo.projection = m_camera.getProjection();
                ubo.view = m_camera.getView();
                ubo.cameraPosition = m_camera.getCameraPos();
                mgr.m_generalMatrixUBO->writeToBuffer(&ubo);
                mgr.m_generalMatrixUBO->flush();

                //render
                m_renderer.beginSwapChainRenderPass(commandBuffer);
                renderObjects(commandBuffer);
                m_renderer.endSwapChainRenderPass(commandBuffer);
                if (m_renderer.endFrame() == true)
                    for (auto& pipeline : mgr.m_pipelines)
                    {
                        Shader prevShader(
                            pipeline.pipeline->getShader().getVertexShaderPath(),
                            pipeline.pipeline->getShader().getFragmentShaderPath()
                        );
                        createPipeline(pipeline.pipelineLayout, pipeline.pipeline, std::move(prevShader));
                    }
            }
        }
        vkDeviceWaitIdle(m_device.device());
    }

    const VkPipelineLayout App::createPipeLineLayout(VkDescriptorSetLayout setLayout) noexcept {
        VkPipelineLayout pipelineLayout;
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
        return pipelineLayout;
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
                .addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                .build();
            
            VkDescriptorSet descriptorSet;
            auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
            auto bufferInfo = uboBuffer->descriptorInfo();

            if (mesh.m_material.m_hasColorMap) {
                auto baseColorPair = mgr.m_textures.try_emplace(mesh.m_material.m_baseColorPath, mesh.m_material.m_baseColorPath);
                m_model->createTexture(baseColorPair.first->second);
                auto textureInfo = baseColorPair.first->second.getDescriptorInfo();

                DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
                    .writeBuffer(0, &globalBufferInfo)
                    .writeBuffer(1, &bufferInfo)
                    .writeImage(2, &textureInfo)
                    .build(descriptorSet);
            }
            else
            {
                DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
                    .writeBuffer(0, &globalBufferInfo)
                    .writeBuffer(1, &bufferInfo)
                    .build(descriptorSet);
            }

            mgr.m_sets.emplace_back(
                std::move(descriptorLayout),
                std::move(uboBuffer),
                descriptorSet
            );

            mesh.m_pipelineId = 0;
            mesh.m_descriptorSetId = mgr.m_sets.size()-1;
            
            //update
            PBRUbo ubo = {
                .modelMatrix = mesh.getModelMatrix(),
                .normalMatrix = glm::transpose(glm::inverse(mesh.getModelMatrix())),
                .baseColor = mesh.m_material.m_baseColor,
                .lightDirection = glm::vec4(1.f, 0.f, 0.f, 0.f),
                .metallic = mesh.m_material.m_metallicFactor,
                .roughness = mesh.m_material.m_roughnessFactor
            };

            auto& buffer = mgr.m_sets[mesh.getDescriptorSetId()].uboBuffer;
            buffer->writeToBuffer(&ubo);
            buffer->flush();
        }
    }

    void App::createPipeline(VkPipelineLayout& pipelineLayout, std::unique_ptr<Pipeline>& pipeline, Shader&& shader) noexcept{
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");

        auto pipeline_config = Pipeline::createDefaultPipeline(m_window.getExtent().width,
            m_window.getExtent().height);
        pipeline_config.renderPass = m_renderer.getSwapChainRenderPass();
        pipeline_config.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(
            m_device,
            std::move(shader),
            pipeline_config);
        LOG_MSG("New Pipeline has been created!");
    }
}  // namespace sge