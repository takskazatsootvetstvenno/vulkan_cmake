#include "App.h"

#include "../../../bindings/imgui_impl_glfw.h"
#include "../../../bindings/imgui_impl_vulkan.h"
#include "Buffer.h"
#include "GLFW/glfw3.h"
#include "Logger.h"
#include "MeshMGR.h"
#include "Shader.h"
#include "Texture.h"
#include "VulkanHelpUtils.h"

#include <array>
#include <chrono>
#include <cstdio>
#include <iterator>
#include <numeric>
#include <vector>

namespace sge {
App::App(glm::ivec2 windowSize, std::string windowName) : m_window(windowSize.x, windowSize.y, std::move(windowName)) {
    initEvents();
    m_camera.setViewCircleCamera(-15.f, 5.f);
    m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()), static_cast<float>(windowSize.x) / windowSize.y,
                                      0.1f, 256.f);

    auto& mgr = MeshMGR::Instance();
    mgr.setDescriptorPool(DescriptorPool::Builder(m_device)
                              .setMaxSets(1024)
                              .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024)
                              .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024)
                              .build());

    mgr.m_generalMatrixUBO = std::make_unique<Buffer>(
        m_device, sizeof(GlobalUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    mgr.m_generalMatrixUBO->map();
    mgr.m_debugUBO = std::make_unique<Buffer>(m_device, sizeof(DebugUBO), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                              VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    mgr.m_debugUBO->map();
    mgr.m_normalTestUBO = std::make_unique<Buffer>(
        m_device, sizeof(NormalTestInfo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
    mgr.m_normalTestUBO->map();
    m_model = std::make_unique<Model>(m_device);
    addSkybox();
    addNormalTestPipeline();
    auto brdfLUT = mgr.m_textures.try_emplace("brdfLUT", "data/brdfLUT.png");
    if (!brdfLUT.first->second.isProcessed()) m_model->createTexture(brdfLUT.first->second);
}

App::~App() {
    auto& mgr = MeshMGR::Instance();
    for (auto& pipeline : mgr.m_pipelines) {
        vkDestroyPipelineLayout(m_device.device(), pipeline.pipelineLayout, nullptr);
    }
    for (auto& textures : mgr.m_textures) {
        vkDestroyImage(m_device.device(), textures.second.getTextureImage(), nullptr);
        vkFreeMemory(m_device.device(), textures.second.getTextureImageMemory(), nullptr);
        vkDestroyImageView(m_device.device(), textures.second.getImageView(), nullptr);
        vkDestroySampler(m_device.device(), textures.second.getSampler(), nullptr);
    }
    mgr.clearTable();
}

void App::renderObjects(VkCommandBuffer commandBuffer) noexcept {
    auto& mgr = MeshMGR::Instance();

    for (auto& mesh : mgr.m_meshes) {
        mgr.m_pipelines[mesh.getPipelineId()].pipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mgr.m_pipelines[mesh.getPipelineId()].pipelineLayout, 0, 1,
                                &mgr.m_sets[mesh.getDescriptorSetId()].set, 0, nullptr);

        m_model->bind(commandBuffer, mesh);
        m_model->draw(commandBuffer, mesh);
    }
    if (m_useNormalPipeline)
        for (auto& mesh : mgr.m_meshes) {
            // update normalTest UBO
            NormalTestInfo normalUBO{.modelMatrix = mesh.getModelMatrix(), .magnitude = m_normalMagnitude};
            mgr.m_normalTestUBO->writeToBuffer(&normalUBO);
            mgr.m_normalTestUBO->flush();

            mgr.m_pipelines[m_normalPipelineID].pipeline->bind(commandBuffer);
            vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                    mgr.m_pipelines[m_normalPipelineID].pipelineLayout, 0, 1,
                                    &mgr.m_sets[m_normalPipelineDescriptorSetID].set, 0, nullptr);

            m_model->bind(commandBuffer, mesh);
            m_model->draw(commandBuffer, mesh);
        }
    for (const auto& systemMesh : mgr.m_systemMeshes) {
        mgr.m_pipelines[systemMesh.getPipelineId()].pipeline->bind(commandBuffer);

        vkCmdBindDescriptorSets(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS,
                                mgr.m_pipelines[systemMesh.getPipelineId()].pipelineLayout, 0, 1,
                                &mgr.m_sets[systemMesh.getDescriptorSetId()].set, 0, nullptr);

        m_model->bind(commandBuffer, systemMesh);
        m_model->draw(commandBuffer, systemMesh);
    }
}

void App::initEvents() noexcept {
    m_eventDispatcher.add_event_listener<EventKeyPressed>([&](EventKeyPressed& event) {
        switch (event.key) {
            case 69:  // E
                m_camera.changeCirclePos(5.f * 3.141592f / 180.f);

                break;
            case 71:  // G
                if (event.action == 0) {
                    auto& mgr = MeshMGR::Instance();
                    auto numOfPipelines = mgr.m_pipelines.size();
                    static int current_pipeline = 0;
                    if (numOfPipelines < 2) break;
                    std::swap(mgr.m_pipelines[current_pipeline % numOfPipelines], mgr.m_pipelines[0]);
                    std::swap(mgr.m_pipelines[0], mgr.m_pipelines[(current_pipeline + 1) % numOfPipelines]);
                    (++current_pipeline) %= numOfPipelines;
                    LOG_MSG("Pipeline name: " << mgr.m_pipelines[0].name << ": "
                                              << mgr.m_pipelines[0].pipeline->getShader().getFragmentShaderPath());
                }
                break;
            case 83:  // S
                m_camera.changeCircleHeight(-1.f);
                break;
            case 87:  // W
                m_camera.changeCircleHeight(1.f);
                break;
            case 81:  // Q
                m_camera.changeCirclePos(-5.f * 3.141592f / 180.f);
                break;
            case 256:  // GLFW_KEY_ESCAPE
                m_window.closeWindow();
                break;
        }
    });
    m_eventDispatcher.add_event_listener<EventScroll>([&](EventScroll& event) {
        m_camera.changeZoom(static_cast<float>(-event.y * 2));
        m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()),
                                          static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height,
                                          0.1f, 100.f);
    });
    m_eventDispatcher.add_event_listener<EventWindowResize>([&](EventWindowResize&) {
        if (m_window.getExtent().width <= 0.1f || m_window.getExtent().height <= 0.1f) return;
        m_camera.setPerspectiveProjection(glm::radians(m_camera.getZoom()),
                                          static_cast<float>(m_window.getExtent().width) / m_window.getExtent().height,
                                          0.1f, 100.f);
    });
    m_window.set_event_callback([&](BaseEvent& event) { m_eventDispatcher.dispatch(event); });
}
void App::addNormalTestPipeline() noexcept {
    auto& mgr = MeshMGR::Instance();
    auto descriptorLayout = DescriptorSetLayout::Builder(m_device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)
                                .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT)
                                .build();
    auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
    auto normalBufferInfo = mgr.m_normalTestUBO->descriptorInfo();
    VkDescriptorSet descriptorSet;
    DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
        .writeBuffer(0, &globalBufferInfo)
        .writeBuffer(1, &normalBufferInfo)
        .build(descriptorSet);

    if (Shader glslNormalShader("data/Shaders/GLSL/Normal/normal.vert", "data/Shaders/GLSL/Normal/normal.frag",
                                "data/Shaders/GLSL/Normal/normal.geom");
        glslNormalShader.isValid()) {
        auto pipelineLayoutSkybox = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
        m_normalPipelineID = mgr.m_pipelines.size();
        mgr.m_pipelines.emplace_back(std::to_string(m_normalPipelineID) + " Normal_test", pipelineLayoutSkybox, nullptr);
        FixedPipelineStates states{.cullingMode = CullingMode::NONE};
        createPipeline(descriptorLayout->getDescriptorSetLayout(), mgr.m_pipelines.back().pipeline,
                       std::move(glslNormalShader),
                       std::move(states));
        LOG_MSG("Pipeline name: " << mgr.m_pipelines.back().name << ": "
                                  << mgr.m_pipelines.back().pipeline->getShader().getFragmentShaderPath());

    }
    mgr.m_sets.emplace_back(std::move(descriptorLayout), nullptr, descriptorSet);
    m_normalPipelineDescriptorSetID = mgr.m_sets.size() - 1;
}
void App::addSkybox() noexcept {
    Texture::CubemapData skyboxInfo{.frontTexturePath = "data/Skybox/front.jpg",
                                    .backTexturePath = "data/Skybox/back.jpg",
                                    .topTexturePath = "data/Skybox/top.jpg",
                                    .bottomTexturePath = "data/Skybox/bottom.jpg",
                                    .leftTexturePath = "data/Skybox/left.jpg",
                                    .rightTexturePath = "data/Skybox/right.jpg"};

    float skyboxVertices[] = {// positions
                              -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  -1.0f,
                              -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f, 1.0f,

                              1.0f,  -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f,

                              -1.0f, -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f, -1.0f, 1.0f,

                              -1.0f, 1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  1.0f,
                              1.0f,  1.0f,  1.0f,  -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f,  -1.0f,

                              -1.0f, -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, -1.0f,
                              1.0f,  -1.0f, -1.0f, -1.0f, -1.0f, 1.0f,  1.0f,  -1.0f, 1.0f};
    auto& mgr = MeshMGR::Instance();
    Mesh skyboxMesh;
    skyboxMesh.m_pos.resize(36);
    for (size_t i = 0; i < sizeof(skyboxVertices) / (3 * sizeof(float)); ++i)
        skyboxMesh.m_pos[i].m_position = {skyboxVertices[3 * i], skyboxVertices[3 * i + 1], skyboxVertices[3 * i + 2]};
    for (size_t i = 0; i < sizeof(skyboxVertices) / (3 * sizeof(float)); ++i) skyboxMesh.m_ind.emplace_back(i);
    auto descriptorLayout = DescriptorSetLayout::Builder(m_device)
                                .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                                .addBinding(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, VK_SHADER_STAGE_FRAGMENT_BIT)
                                .build();
    auto skyboxPair = mgr.m_textures.try_emplace("skybox", std::move(skyboxInfo));
    if (!skyboxPair.first->second.isProcessed()) m_model->createTexture(skyboxPair.first->second);
    auto skyboxDescriptorInfo = skyboxPair.first->second.getDescriptorInfo();
    auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
    VkDescriptorSet descriptorSet;
    DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
        .writeBuffer(0, &globalBufferInfo)
        .writeImage(8, &skyboxDescriptorInfo)
        .build(descriptorSet);

    if (Shader glslSkyboxShader("data/Shaders/GLSL/Skybox/skybox.vert", "data/Shaders/GLSL/Skybox/skybox.frag");
        glslSkyboxShader.isValid()) {
        auto pipelineLayoutSkybox = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " skybox_GLSL", pipelineLayoutSkybox,
                                     nullptr);
        FixedPipelineStates states{.depthWriteEnable = false, .depthOp = CompareOp::LESS_OR_EQUAL};
        createPipeline(pipelineLayoutSkybox, mgr.m_pipelines.back().pipeline, std::move(glslSkyboxShader), states);
        skyboxMesh.m_pipelineId = static_cast<uint32_t>(mgr.m_pipelines.size() - 1);
        LOG_MSG("Pipeline name: " << mgr.m_pipelines.back().name << ": "
                                  << mgr.m_pipelines.back().pipeline->getShader().getFragmentShaderPath());
    }
    mgr.m_sets.emplace_back(std::move(descriptorLayout), nullptr, descriptorSet);
    skyboxMesh.m_descriptorSetId = static_cast<uint32_t>(mgr.m_sets.size() - 1);
    mgr.m_systemMeshes.emplace_back(std::move(skyboxMesh));
    Texture::CubemapData skyboxIrradiance{.frontTexturePath = "data/Skybox/Irradiance/0.bmp",
                                          .backTexturePath = "data/Skybox/Irradiance/1.bmp",
                                          .topTexturePath = "data/Skybox/Irradiance/3.bmp",
                                          .bottomTexturePath = "data/Skybox/Irradiance/2.bmp",
                                          .leftTexturePath = "data/Skybox/Irradiance/4.bmp",
                                          .rightTexturePath = "data/Skybox/Irradiance/5.bmp"};
    auto envIrradiancePair = mgr.m_textures.try_emplace("skybox_irradiance", std::move(skyboxIrradiance));
    if (!envIrradiancePair.first->second.isProcessed()) m_model->createTexture(envIrradiancePair.first->second);
}
void App::run() {
    m_model->createBuffers();
    auto& mgr = MeshMGR::Instance();
    init_imgui();
    const std::array table{
        "Default", "Shader normal", "Base color", "Normal", "Occlusion", "Emissive", "Metallic", "Roughness",
    };
    int currentItem = 0;
    while (!m_window.shouldClose()) {
        glfwPollEvents();
        ImGui_ImplVulkan_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();
        ImGui::SetNextWindowPos(ImVec2(ImGui::GetIO().DisplaySize.x * 0.25f, ImGui::GetIO().DisplaySize.y * 0.25f),
                                ImGuiCond_FirstUseEver, ImVec2(0.5f, 0.5f));
        ImGui::Begin("Debug window");
        ImGui::ListBox("Shader Output", &currentItem, table.data(), table.size());
        ImGui::SetNextItemOpen(true, ImGuiCond_Once);
        if (ImGui::TreeNode("Pipelines")) {
            for (const auto& pipeline : mgr.m_pipelines) {
                if (ImGui::TreeNode(pipeline.name.c_str())) {
                    const auto& shader = pipeline.pipeline->getShader();
                    ImGui::Separator();
                    if (ImGui::Button("Recreate pipeline")) pipeline.pipeline->recreatePipelineShaders();
                    ImGui::Separator();
                    ImGui::Text("%s", std::string("Vertex shader path:\n" + shader.getVertexShaderPath()).c_str());
                    ImGui::Text("%s", std::string("Fragment shader path:\n" + shader.getFragmentShaderPath()).c_str());
                    if (shader.isGeometryShaderPresent())
                        ImGui::Text("%s",
                                    std::string("Fragment shader path:\n" + shader.getGeometryShaderPath()).c_str());
                    ImGui::Separator();
                    ImGui::Text("%s", std::string("Vert defines:\n" + shader.getDefines().vertShaderDefines).c_str());
                    ImGui::Text("%s",
                                std::string("Frag defines:\n" + shader.getDefines().fragmentShaderDefines).c_str());
                    if (shader.isGeometryShaderPresent())
                        ImGui::Text("%s",
                                    std::string("Geom defines:\n" + shader.getDefines().geometryShaderDefines).c_str());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        if (ImGui::TreeNode("Use normal-test pipeline")) {
            m_useNormalPipeline = true;
            ImGui::SliderFloat("NormalMagnitude:", &m_normalMagnitude, 0.01f, 10.f);
            ImGui::TreePop();
        } else
            m_useNormalPipeline = false;

        ImGui::Text("%s", (std::string("Camera position: \n") + std::to_string(m_camera.getCameraPos().x) + " " +
                           std::to_string(m_camera.getCameraPos().y) + " " + std::to_string(m_camera.getCameraPos().z))
                              .c_str());
        if (ImGui::TreeNode(std::string("Meshes (" + std::to_string(mgr.m_meshes.size()) + ")").c_str())) {
            for (const auto& mesh : mgr.m_meshes) {
                if (ImGui::TreeNode(mesh.getName().c_str())) {
                    ImGui::Text("%s", std::string("Pipeline ID: " + std::to_string(mesh.getPipelineId())).c_str());
                    ImGui::Text("%s",
                                std::string("DesctiptorSet ID: " + std::to_string(mesh.getDescriptorSetId())).c_str());
                    ImGui::Text("%s", std::string("Num of vertices: " + std::to_string(mesh.getVertexCount())).c_str());
                    ImGui::TreePop();
                }
            }
            ImGui::TreePop();
        }
        ImGui::End();
        ImGui::Render();
        if (auto commandBuffer = m_renderer.beginFrame()) {
            // update global variables
            GlobalUbo ubo{.projection = m_camera.getProjection(),
                          .view = m_camera.getView(),
                          .cameraPosition = m_camera.getCameraPos()};
            mgr.m_generalMatrixUBO->writeToBuffer(&ubo);
            mgr.m_generalMatrixUBO->flush();

            // update debug UBO
            DebugUBO debugUBO{.outType = static_cast<unsigned int>(currentItem)};
            mgr.m_debugUBO->writeToBuffer(&debugUBO);
            mgr.m_debugUBO->flush();

            // render
            m_renderer.beginSwapChainRenderPass(commandBuffer);
            renderObjects(commandBuffer);
            ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
            m_renderer.endSwapChainRenderPass(commandBuffer);
            if (m_renderer.endFrame() == true)
                for (auto& pipeline : mgr.m_pipelines) {
                    Shader prevShader = pipeline.pipeline->getShader();
                    createPipeline(pipeline.pipelineLayout, pipeline.pipeline, std::move(prevShader));
                }
        }
    }
    vkDeviceWaitIdle(m_device.device());
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}



const VkPipelineLayout App::createPipeLineLayout(VkDescriptorSetLayout setLayout) noexcept {
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{setLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    auto result = vkCreatePipelineLayout(m_device.device(), &pipelineLayoutInfo, nullptr, &pipelineLayout);
    VK_CHECK_RESULT(result, "Failed to create pipeline layout");
    return pipelineLayout;
}

void App::init_imgui() {
    IMGUI_CHECKVERSION();

    MeshMGR::Instance().m_UIPool = DescriptorPool::Builder(m_device)
                                       .setMaxSets(1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLER, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1024)
                                       .addPoolSize(VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1024)
                                       .build();
    // 2: initialize imgui library

    // this initializes the core structures of imgui
    ImGui::CreateContext();

    // this initializes imgui for SDL
    ImGui_ImplGlfw_InitForVulkan(m_window.getHandle(), true);

    // this initializes imgui for Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = m_device.getInstance();
    init_info.PhysicalDevice = m_device.getPhysicalDevice();
    init_info.Device = m_device.device();
    init_info.Queue = m_device.graphicsQueue();
    init_info.DescriptorPool = MeshMGR::Instance().m_UIPool->getDescriptorPool();
    init_info.MinImageCount = 3;
    init_info.ImageCount = 3;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    auto CB = m_device.beginSingleTimeCommands();
    ImGui_ImplVulkan_Init(&init_info, m_renderer.getSwapChainRenderPass());

    // execute a gpu command to upload imgui font textures
    ImGui_ImplVulkan_CreateFontsTexture(CB);
    m_device.endSingleTimeCommands(CB);

    // clear font textures from cpu data
    ImGui_ImplVulkan_DestroyFontUploadObjects();
}

void App::loadModels(std::vector<Mesh>&& meshess) {
    auto& mgr = MeshMGR::Instance();
    auto& mgr_meshes = mgr.m_meshes;

    for (auto& mesh : meshess) {
        auto uboBuffer = std::make_unique<Buffer>(m_device, sizeof(PBRUbo), 1, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                                  VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT);
        uboBuffer->map();

        auto descriptorLayoutBuilder = DescriptorSetLayout::Builder(m_device)
                                           .addBinding(0, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, VK_SHADER_STAGE_VERTEX_BIT)
                                           .addBinding(1, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                                       VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT)
                                           .addBinding(
                                               100, VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
                                               VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT);  // debug

        if (mesh.m_material.m_hasColorMap)
            descriptorLayoutBuilder.addBinding(2, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
        if (mesh.m_material.m_hasMetallicRoughnessMap)
            descriptorLayoutBuilder.addBinding(3, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
        if (mesh.m_material.m_hasNormalMap)
            descriptorLayoutBuilder.addBinding(4, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               VK_SHADER_STAGE_FRAGMENT_BIT);
        if (mesh.m_material.m_hasEmissiveMap)
            descriptorLayoutBuilder.addBinding(5, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                               VK_SHADER_STAGE_FRAGMENT_BIT);

        descriptorLayoutBuilder.addBinding(8, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                           VK_SHADER_STAGE_FRAGMENT_BIT);  // skybox map
        descriptorLayoutBuilder.addBinding(9, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                           VK_SHADER_STAGE_FRAGMENT_BIT);  // skybox irradiance map
        descriptorLayoutBuilder.addBinding(10, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
                                           VK_SHADER_STAGE_FRAGMENT_BIT);  // brdfLUT

        auto descriptorLayout = descriptorLayoutBuilder.build();

        auto globalBufferInfo = mgr.m_generalMatrixUBO->descriptorInfo();
        auto bufferInfo = uboBuffer->descriptorInfo();
        auto debuggerBufferInfo = mgr.m_debugUBO->descriptorInfo();
        std::string defines;
        auto DW = DescriptorWriter(*descriptorLayout, mgr.getDescriptorPool())
                      .writeBuffer(0, &globalBufferInfo)
                      .writeBuffer(1, &bufferInfo)
                      .writeBuffer(100, &debuggerBufferInfo);  // debug
        {
            VkDescriptorImageInfo baseColorDescriptorImageInfo;
            VkDescriptorImageInfo MetallicRoughnessDescriptorImageInfo;
            VkDescriptorImageInfo NormalDescriptorImageInfo;
            VkDescriptorImageInfo EmissiveDescriptorImageInfo;
            VkDescriptorImageInfo skyboxDescriptorInfo;
            VkDescriptorImageInfo skyboxIrradianceDescriptorInfo;
            VkDescriptorImageInfo brdfLUTDescriptorInfo;
            if (mesh.m_material.m_hasColorMap) {
                auto baseColorPair = mgr.m_textures.try_emplace(mesh.m_material.m_baseColorPath,
                                                                mesh.m_material.m_baseColorPath);
                if (!baseColorPair.first->second.isProcessed()) m_model->createTexture(baseColorPair.first->second);
                baseColorDescriptorImageInfo = baseColorPair.first->second.getDescriptorInfo();
                DW.writeImage(2, &baseColorDescriptorImageInfo);
                defines += "#define HAS_COLOR_MAP\n";
            }
            if (mesh.m_material.m_hasMetallicRoughnessMap) {
                auto metallicRoughnessPair = mgr.m_textures.try_emplace(mesh.m_material.m_MetallicRoughnessPath,
                                                                        mesh.m_material.m_MetallicRoughnessPath);
                if (!metallicRoughnessPair.first->second.isProcessed())
                    m_model->createTexture(metallicRoughnessPair.first->second);
                MetallicRoughnessDescriptorImageInfo = metallicRoughnessPair.first->second.getDescriptorInfo();
                DW.writeImage(3, &MetallicRoughnessDescriptorImageInfo);
                defines += "#define HAS_METALLIC_ROUGHNESS_MAP\n";
            }
            if (mesh.m_material.m_hasNormalMap) {
                auto normalPair = mgr.m_textures.try_emplace(mesh.m_material.m_NormalPath, mesh.m_material.m_NormalPath);
                if (!normalPair.first->second.isProcessed()) m_model->createTexture(normalPair.first->second);
                NormalDescriptorImageInfo = normalPair.first->second.getDescriptorInfo();
                DW.writeImage(4, &NormalDescriptorImageInfo);
                defines += "#define HAS_NORMAL_MAP\n";
            }
            if (mesh.m_material.m_hasEmissiveMap) {
                auto emissivePair = mgr.m_textures.try_emplace(mesh.m_material.m_EmissivePath,
                                                               mesh.m_material.m_EmissivePath);
                if (!emissivePair.first->second.isProcessed()) m_model->createTexture(emissivePair.first->second);
                EmissiveDescriptorImageInfo = emissivePair.first->second.getDescriptorInfo();
                DW.writeImage(5, &EmissiveDescriptorImageInfo);
                defines += "#define HAS_EMISSIVE_MAP\n";
            }
            if (mesh.m_material.m_hasOcclusionMap) defines += "#define HAS_OCCLUSION_MAP\n";
            {
                auto skyboxPair = mgr.m_textures.find("skybox");
                if (skyboxPair == mgr.m_textures.end()) {
                    LOG_ERROR("Skybox texture is missing!");
                    assert(false && "Skybox texture is missing!");
                }
                skyboxDescriptorInfo = skyboxPair->second.getDescriptorInfo();
                DW.writeImage(8, &skyboxDescriptorInfo);

                auto skyboxIrradiancePair = mgr.m_textures.find("skybox_irradiance");
                if (skyboxIrradiancePair == mgr.m_textures.end()) {
                    LOG_ERROR("skybox_irradiance texture is missing!");
                    assert(false && "skybox_irradiance texture is missing!");
                }
                skyboxIrradianceDescriptorInfo = skyboxIrradiancePair->second.getDescriptorInfo();
                DW.writeImage(9, &skyboxIrradianceDescriptorInfo);

                auto brdfLUT = mgr.m_textures.find("brdfLUT");
                if (brdfLUT == mgr.m_textures.end()) {
                    LOG_ERROR("brdfLUT texture is missing!");
                    assert(false && "brdfLUT texture is missing!");
                }
                brdfLUTDescriptorInfo = brdfLUT->second.getDescriptorInfo();
                DW.writeImage(10, &brdfLUTDescriptorInfo);
            }
        }
        VkDescriptorSet descriptorSet;
        DW.build(descriptorSet);
        switch (mesh.m_materialType) {
            case Mesh::MaterialType::Phong: defines += "#define Phong\n"; break;
            case Mesh::MaterialType::PBR: defines += "#define PBR\n"; break;
        }
        mesh.m_pipelineId = -1;

        for (size_t i = 0; i < mgr.m_pipelines.size(); ++i) {
            if (auto& def = mgr.m_pipelines[i].pipeline->getShader().getDefines();
                def.fragmentShaderDefines == defines && def.vertShaderDefines == "")
                mesh.m_pipelineId = i;
        }
        if (mesh.m_pipelineId == -1) {
            switch (mesh.m_materialType) {
                case Mesh::MaterialType::Phong:
                    if (Shader glslPhongShader("data/Shaders/GLSL/Phong/phong.vert",
                                               "data/Shaders/GLSL/Phong/phong.frag", "", {"", defines});
                        glslPhongShader.isValid()) {
                        auto pipelineLayoutGLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " Phong_GLSL",
                                                     pipelineLayoutGLSLPhong, nullptr);
                        mesh.m_pipelineId = mgr.m_pipelines.size() - 1;
                        createPipeline(descriptorLayout->getDescriptorSetLayout(), mgr.m_pipelines.back().pipeline,
                                       std::move(glslPhongShader));
                        LOG_MSG("Pipeline name: "
                                << mgr.m_pipelines[mesh.m_pipelineId].name << ": "
                                << mgr.m_pipelines[mesh.m_pipelineId].pipeline->getShader().getFragmentShaderPath());
                    }
                    break;
                case Mesh::MaterialType::PBR:
                    if (Shader glslPBRShader("data/Shaders/GLSL/PBR/PBR.vert", "data/Shaders/GLSL/PBR/PBR.frag", "",
                                             {"", defines});
                        glslPBRShader.isValid()) {
                        auto pipelineLayoutGLSLPBR = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " PBR_GLSL",
                                                     pipelineLayoutGLSLPBR, nullptr);
                        mesh.m_pipelineId = mgr.m_pipelines.size() - 1;
                        FixedPipelineStates states{};
                        states.cullingMode = CullingMode::BACK;
                        createPipeline(descriptorLayout->getDescriptorSetLayout(), mgr.m_pipelines.back().pipeline,
                                       std::move(glslPBRShader),
                                       std::move(states));
                        LOG_MSG("Pipeline name: "
                                << mgr.m_pipelines[mesh.m_pipelineId].name << ": "
                                << mgr.m_pipelines[mesh.m_pipelineId].pipeline->getShader().getFragmentShaderPath());
                    }
#if 0
                    if (Shader glslPhongShader("data/Shaders/GLSL/Phong/phong.vert", "data/Shaders/GLSL/Phong/phong.frag", { "", defines + "#define TESTPHONG" }); glslPhongShader.isValid())
                    {
                        auto pipelineLayoutGLSLPhong = createPipeLineLayout(descriptorLayout->getDescriptorSetLayout());
                        mgr.m_pipelines.emplace_back(std::to_string(mgr.m_pipelines.size()) + " Phong_GLSL", pipelineLayoutGLSLPhong, nullptr);
                        createPipeline(descriptorLayout->getDescriptorSetLayout(), mgr.m_pipelines.back().pipeline, std::move(glslPhongShader));
                    }
#endif
                    break;
            }
        }

        mgr.m_sets.emplace_back(std::move(descriptorLayout), std::move(uboBuffer), descriptorSet);

        mesh.m_descriptorSetId = mgr.m_sets.size() - 1;

        // update
        PBRUbo ubo = {.modelMatrix = mesh.getModelMatrix(),
                      .normalMatrix = glm::transpose(glm::inverse(mesh.getModelMatrix())),
                      .baseColor = mesh.m_material.m_baseColor,
                      .lightDirection = glm::vec4(1.f, 0.f, 0.f, 0.f),
                      .metallic = mesh.m_material.m_metallicFactor,
                      .roughness = mesh.m_material.m_roughnessFactor};

        auto& buffer = mgr.m_sets[mesh.getDescriptorSetId()].uboBuffer;
        buffer->writeToBuffer(&ubo);
        buffer->flush();
    }
    mgr_meshes.insert(mgr_meshes.end(), std::make_move_iterator(meshess.begin()),
                      std::make_move_iterator(meshess.end()));
}

void App::createPipeline(const VkDescriptorSetLayout descriptorSetLayout, std::unique_ptr<Pipeline>& pipeline,
                         Shader&& shader, FixedPipelineStates states) {
    PipelineInputData::VertexData vertexData(Vertex::getBindingDescription(), Vertex::getAttributeDescription());
    PipelineInputData::ColorBlendData colorBlendData(Pipeline::createDefaultColorAttachments());
    auto pipelineLayout = Pipeline::createPipeLineLayout(m_device.device(), descriptorSetLayout);
    PipelineInputData::FixedFunctionsStages fixedFunctionStages(m_window.getExtent().width, m_window.getExtent().height);
    auto renderPass = m_renderer.getSwapChainRenderPass();

    PipelineInputData pipeline_data {
        vertexData,
        shader,
        colorBlendData,
        pipelineLayout,
        fixedFunctionStages,
        renderPass
    };
    pipeline = std::make_unique<Pipeline>(m_device, std::move(pipeline_data));
}

void App::createPipeline(const VkPipelineLayout pipelineLayout, std::unique_ptr<Pipeline>& pipeline, Shader&& shader,
    FixedPipelineStates states) {
    PipelineInputData::VertexData vertexData(Vertex::getBindingDescription(), Vertex::getAttributeDescription());
    PipelineInputData::ColorBlendData colorBlendData(Pipeline::createDefaultColorAttachments());
    PipelineInputData::FixedFunctionsStages fixedFunctionStages(m_window.getExtent().width, m_window.getExtent().height);
    auto renderPass = m_renderer.getSwapChainRenderPass();

    PipelineInputData pipeline_data{vertexData,          shader,    colorBlendData, pipelineLayout,
                                    fixedFunctionStages, renderPass};
    pipeline = std::make_unique<Pipeline>(m_device, std::move(pipeline_data));
}

}  // namespace sge
