#pragma once
#include "Window.h"
#include "Device.h"
#include "Renderer.h"
#include "Pipeline.h"
#include "model.h"
#include <memory>
namespace sge {
    class App {
    public:
        App();
        ~App();
        App(const App&) = delete;
        App& operator=(const App&) = delete;

        void run();
    private:
        void loadModels();
        void createPipeline();
        void createPipeLineLayout();
        void renderGameObjects(VkCommandBuffer commandBuffer);
        void recordCommandBuffer(const int imageIndex);

        Window m_window{800, 600, "vulkan_window"};
        Device m_device{m_window};
        Renderer m_renderer{ m_window, m_device };
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<Pipeline> m_pipeline;
        std::unique_ptr<Model> m_model;
    };

} 
