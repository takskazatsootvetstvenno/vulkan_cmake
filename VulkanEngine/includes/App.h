#pragma once
#include "Event.h"
#include "Window.h"
#include "Device.h"
#include "Renderer.h"
#include "Pipeline.h"
#include "Model.h"
#include "Camera.h"
#include "Descriptors.h"
#include <memory>

namespace sge {
    struct GlobalUbo
    {
        glm::mat4 projection{ 1.f };
        glm::mat4 view{ 1.f };
        glm::vec3 cameraPosition{ 0.f };
    };

    struct PBRUbo
    {
        glm::mat4 modelMatrix{ 1.f };
        glm::mat3 normalMatrix{ 1.f };
        glm::vec4 baseColor{ 1.f };
        glm::vec4 lightDirection{ 0.f }; //alignas(16)
        float metallic = 0.f;
        float roughness = 0.f;
    };
    
    class App {
    public:
        App();
        ~App();
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
        void loadModels(std::vector<Mesh>&& meshes);
    private:

        void createPipeline(VkPipelineLayout& pipelineLayout, std::unique_ptr<Pipeline>& pipeline, Shader&& shader) noexcept;
        const VkPipelineLayout createPipeLineLayout(VkDescriptorSetLayout setLayout) noexcept;
        void renderObjects(VkCommandBuffer commandBuffer) noexcept;
        void initEvents() noexcept;
        Window m_window{800, 600, "vulkan_window"};
        Device m_device{m_window};
        Renderer m_renderer{ m_window, m_device };
        std::unique_ptr<Model> m_model;
        Camera m_camera;
        EventDispatcher m_eventDispatcher;
    };

} 
