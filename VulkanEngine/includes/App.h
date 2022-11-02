#pragma once
#include "Camera.h"
#include "Descriptors.h"
#include "Device.h"
#include "Event.h"
#include "Model.h"
#include "Pipeline.h"
#include "Renderer.h"
#include "Window.h"

#include <memory>

namespace sge {
struct GlobalUbo {
    glm::mat4 projection{1.f};
    glm::mat4 view{1.f};
    glm::vec3 cameraPosition{0.f};
};

struct DebugUBO {
    unsigned int outType = 0;
};

struct NormalTestInfo {
    glm::mat4 modelMatrix = glm::mat4{};
    float magnitude = 1.f;
};

struct PBRUbo {
    glm::mat4 modelMatrix{1.f};
    glm::mat3 normalMatrix{1.f};
    glm::vec4 baseColor{1.f};
    glm::vec4 lightDirection{0.f};  // alignas(16)
    float metallic = 0.f;
    float roughness = 0.f;
};

class App {
 public:
    App(glm::ivec2 windowSize, std::string windowName);
    ~App();
    App(const App&) = delete;
    App& operator=(const App&) = delete;

    void run();
    void loadModels(std::vector<Mesh>&& meshes);

 private:
    void createPipeline(VkPipelineLayout& pipelineLayout, std::unique_ptr<Pipeline>& pipeline, Shader&& shader,
                        FixedPipelineStates states = FixedPipelineStates()) noexcept;
    const VkPipelineLayout createPipeLineLayout(VkDescriptorSetLayout setLayout) noexcept;
    void renderObjects(VkCommandBuffer commandBuffer) noexcept;
    void initEvents() noexcept;
    void addSkybox() noexcept;
    void addNormalTestPipeline() noexcept;
    void init_imgui();
    Window m_window{800, 600, "vulkan_window"};
    Device m_device{m_window};
    Renderer m_renderer{m_window, m_device};
    std::unique_ptr<Model> m_model;
    Camera m_camera;
    EventDispatcher m_eventDispatcher;
    bool m_useNormalPipeline = false;
    size_t m_normalPipelineID = -1;
    size_t m_normalPipelineDescriptorSetID = 0;
    float m_normalMagnitude = 0.2f;
};

}  // namespace sge
