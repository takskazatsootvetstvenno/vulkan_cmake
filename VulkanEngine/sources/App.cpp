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
    App::App(glm::ivec2 windowSize, std::string windowName)
        :m_window(windowSize.x, windowSize.y, std::move(windowName))
    {
        m_camera.setViewCircleCamera(15.f, 5.f);
        m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(windowSize.x) / windowSize.y, 0.1f, 100.f);
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
        m_model = std::make_unique<Model>(m_device);
        addSkybox();
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
        for (auto& systemMesh : mgr.m_systemMeshes)
        {
            mgr.m_pipelines[systemMesh.getPipelineId()].pipeline->bind(commandBuffer);

            vkCmdBindDescriptorSets(
                commandBuffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                mgr.m_pipelines[systemMesh.getPipelineId()].pipelineLayout,
                0,
                1,
                &mgr.m_sets[systemMesh.getDescriptorSetId()].set,
                0,
                nullptr);

            m_model->bind(commandBuffer, systemMesh);
            m_model->draw(commandBuffer, systemMesh);
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
                        LOG_MSG("Pipeline name: " << mgr.m_pipelines[0].name << ": " << mgr.m_pipelines[0].pipeline->getShader().getFragmentShaderPath());
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
    void App::addSkybox() noexcept
    {
        Texture::CubemapData skyboxInfo
        {
            .frontTexturePath = "Skybox/front.jpg",
            .backTexturePath = "Skybox/back.jpg",
            .topTexturePath = "Skybox/top.jpg",
            .bottomTexturePath = "Skybox/bottom.jpg",
            .leftTexturePath = "Skybox/left.jpg",
            .rightTexturePath = "Skybox/right.jpg"
        };
        float skyboxVertices[] = {
            // positions          
            -1.0f,  1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f, -1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,

            -1.0f, -1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f, -1.0f,  1.0f,
            -1.0f, -1.0f,  1.0f,

            -1.0f,  1.0f, -1.0f,
             1.0f,  1.0f, -1.0f,
             1.0f,  1.0f,  1.0f,
             1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f,  1.0f,
            -1.0f,  1.0f, -1.0f,

            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f, -1.0f,
             1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f,  1.0f,
             1.0f, -1.0f,  1.0f
        };
        auto& mgr = MeshMGR::Instance();
        Mesh skyboxMesh;
        skyboxMesh.m_pos.resize(36);
        for (int i = 0; i < sizeof(skyboxVertices) / (3 * sizeof(float)); ++i)
            skyboxMesh.m_pos[i].m_position = { skyboxVertices[3 * i], skyboxVertices[3 * i + 1], skyboxVertices[3 * i + 2] };
        for (int i = 0; i < sizeof(skyboxVertices) / (3 * sizeof(float)); ++i)
            skyboxMesh.m_ind.emplace_back(i);
        auto descriptorLayout = DescriptorSetLayout::Builder(m_device)
            .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
            .addBinding(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
            .build();
        auto skyboxPair = mgr.m_textures.try_emplace("skybox", std::move(skyboxInfo));
        if (!skyboxPair.first->second.isProcessed())
            m_model->createTexture(skyboxPair.first->second);
        auto skyboxDescriptorInfo = skyboxPair.first->second.getDescriptorInfo();
        auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
        VkDescriptorSet descriptorSet;
        auto DW = DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
            .writeBuffer(0, &globalBufferInfo)
            .writeImage(8, &skyboxDescriptorInfo)
            .build(descriptorSet);

        if (Shader glslSkyboxShader("Shaders/GLSL/Skybox/skybox.vert", "Shaders/GLSL/Skybox/skybox.frag", { "", "" }); glslSkyboxShader.isValid())
        {
            auto pipelineLayoutSkybox = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
            mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " skybox_GLSL", pipelineLayoutSkybox, nullptr);
            FixedPipelineStates states{
                .depthOp = VK_COMPARE_OP_LESS_OR_EQUAL,
                .depthWriteEnable = false
            };
            createPipeline(pipelineLayoutSkybox, mgr.m_pipelines.back().pipeline, std::move(glslSkyboxShader), states);
            skyboxMesh.m_pipelineId = mgr.m_pipelines.size() - 1;
            LOG_MSG("Pipeline name: " << mgr.m_pipelines.back().name << ": " << mgr.m_pipelines.back().pipeline->getShader().getFragmentShaderPath());
        }
        mgr.m_sets.emplace_back(
            std::move(descriptorLayout),
            nullptr,
            descriptorSet
        );
        skyboxMesh.m_descriptorSetId = mgr.m_sets.size() - 1;
        mgr.m_systemMeshes.emplace_back(std::move(skyboxMesh));
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
                        Shader prevShader = pipeline.pipeline->getShader();
                        createPipeline(pipeline.pipelineLayout, pipeline.pipeline, std::move(prevShader), pipeline.pipeline->getPipelineStates());
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

        for (auto& mesh : meshess)
        {
            auto uboBuffer = std::make_unique<Buffer>(
                m_device,
                sizeof(PBRUbo),
                1,
                VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT
            );
            uboBuffer->map();

            auto descriptorLayoutBuilder = DescriptorSetLayout::Builder(m_device)
                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);

            if (mesh.m_material.m_hasColorMap) 
                descriptorLayoutBuilder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            if (mesh.m_material.m_hasMetallicRoughnessMap)
                descriptorLayoutBuilder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            if (mesh.m_material.m_hasNormalMap)
                descriptorLayoutBuilder.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);
            if (mesh.m_material.m_hasEmissiveMap)
                descriptorLayoutBuilder.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT);

            auto descriptorLayout = descriptorLayoutBuilder.build();

            auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
            auto bufferInfo = uboBuffer->descriptorInfo();

            auto DW = DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
                .writeBuffer(0, &globalBufferInfo)
                .writeBuffer(1, &bufferInfo);

            std::string defines;
            if (mesh.m_material.m_hasColorMap) {
                auto baseColorPair = mgr.m_textures.try_emplace(mesh.m_material.m_baseColorPath, mesh.m_material.m_baseColorPath);
                if(!baseColorPair.first->second.isProcessed())
                    m_model->createTexture(baseColorPair.first->second);
                auto textureInfo = baseColorPair.first->second.getDescriptorInfo();
                DW.writeImage(2, &textureInfo);
                defines += "#define HAS_COLOR_MAP\n";
            }
            if (mesh.m_material.m_hasMetallicRoughnessMap) {
                auto metallicRoughnessPair = mgr.m_textures.try_emplace(mesh.m_material.m_MetallicRoughnessPath, mesh.m_material.m_MetallicRoughnessPath);
                if (!metallicRoughnessPair.first->second.isProcessed())
                    m_model->createTexture(metallicRoughnessPair.first->second);
                auto textureInfo = metallicRoughnessPair.first->second.getDescriptorInfo();
                DW.writeImage(3, &textureInfo);
                defines += "#define HAS_METALLIC_ROUGHNESS_MAP\n";
            }

            if (mesh.m_material.m_hasNormalMap) {
                auto normalPair = mgr.m_textures.try_emplace(mesh.m_material.m_NormalPath, mesh.m_material.m_NormalPath);
                if (!normalPair.first->second.isProcessed())
                    m_model->createTexture(normalPair.first->second);
                auto textureInfo = normalPair.first->second.getDescriptorInfo();
                DW.writeImage(4, &textureInfo);
                defines += "#define HAS_NORMAL_MAP\n";
            }

            if (mesh.m_material.m_hasEmissiveMap) {
                auto emissivePair = mgr.m_textures.try_emplace(mesh.m_material.m_EmissivePath, mesh.m_material.m_EmissivePath);
                if (!emissivePair.first->second.isProcessed())
                    m_model->createTexture(emissivePair.first->second);
                auto textureInfo = emissivePair.first->second.getDescriptorInfo();
                DW.writeImage(5, &textureInfo);
                defines += "#define HAS_EMISSIVE_MAP\n";
            }
            switch (mesh.m_materialType)
            {
            case Mesh::MaterialType::Phong:
                defines += "#define Phong\n";
                break;
            case Mesh::MaterialType::PBR:
                defines += "#define PBR\n";
                break;
            }

            VkDescriptorSet descriptorSet;
            DW.build(descriptorSet);
            mesh.m_pipelineId = -1;
            
            for(int i = 0; i < mgr.m_pipelines.size(); ++i)
            {
                if(auto& def = mgr.m_pipelines[i].pipeline->getShader().getDefines();
                    def.fragmentShaderDefines == defines && def.vertShaderDefines == "")
                    mesh.m_pipelineId = i;
            }
            if (mesh.m_pipelineId == -1)
            {
                switch (mesh.m_materialType)
                {
                case Mesh::MaterialType::Phong:
                    if (Shader glslPhongShader("Shaders/GLSL/Phong/phong.vert", "Shaders/GLSL/Phong/phong.frag", { "", defines}); glslPhongShader.isValid())
                    {
                        auto pipelineLayoutGLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " Phong_GLSL", pipelineLayoutGLSLPhong, nullptr);
                        mesh.m_pipelineId = mgr.m_pipelines.size() - 1;
                        createPipeline(pipelineLayoutGLSLPhong, mgr.m_pipelines.back().pipeline, std::move(glslPhongShader));
                        LOG_MSG("Pipeline name: " << mgr.m_pipelines[mesh.m_pipelineId].name << ": " << mgr.m_pipelines[mesh.m_pipelineId].pipeline->getShader().getFragmentShaderPath());
                    }
                    break;
                case Mesh::MaterialType::PBR:
                    if (Shader glslPBRShader("Shaders/GLSL/PBR/PBR.vert", "Shaders/GLSL/PBR/PBR.frag", { "", defines}); glslPBRShader.isValid())
                    {
                        auto pipelineLayoutGLSLPBR = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " PBR_GLSL", pipelineLayoutGLSLPBR, nullptr);
                        mesh.m_pipelineId = mgr.m_pipelines.size() - 1;
                        createPipeline(pipelineLayoutGLSLPBR, mgr.m_pipelines.back().pipeline, std::move(glslPBRShader));
                        LOG_MSG("Pipeline name: " << mgr.m_pipelines[mesh.m_pipelineId].name << ": " << mgr.m_pipelines[mesh.m_pipelineId].pipeline->getShader().getFragmentShaderPath());
                    }
#if 1
                    if (Shader glslPhongShader("Shaders/GLSL/Phong/phong.vert", "Shaders/GLSL/Phong/phong.frag", { "", defines + "#define TESTPHONG" }); glslPhongShader.isValid())
                    {
                        auto pipelineLayoutGLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " Phong_GLSL", pipelineLayoutGLSLPhong, nullptr);
                        createPipeline(pipelineLayoutGLSLPhong, mgr.m_pipelines.back().pipeline, std::move(glslPhongShader));
                    }
#endif
                    break;
                }
            }

            mgr.m_sets.emplace_back(
                std::move(descriptorLayout),
                std::move(uboBuffer),
                descriptorSet
            );
            
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
        mgr_meshes.insert(mgr_meshes.end(),
            std::make_move_iterator(meshess.begin()),
            std::make_move_iterator(meshess.end()));
    }

    void App::createPipeline(VkPipelineLayout& pipelineLayout, std::unique_ptr<Pipeline>& pipeline, Shader&& shader, FixedPipelineStates states) noexcept{
        assert(pipelineLayout != nullptr && "Cannot create pipeline before pipeline layout");
        assert(shader.isValid() != false && "Cannot create pipeline with invalid shader");
        
        auto pipeline_config = Pipeline::createDefaultPipeline(m_window.getExtent().width,
            m_window.getExtent().height, std::move(states));

        pipeline_config.renderPass = m_renderer.getSwapChainRenderPass();
        pipeline_config.pipelineLayout = pipelineLayout;
        pipeline = std::make_unique<Pipeline>(
            m_device,
            std::move(shader),
            pipeline_config);
        LOG_MSG("New Pipeline has been created!");
    }
}  // namespace sge