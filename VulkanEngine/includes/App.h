#pragma once
#include "Window.h"
#include "Device.h"
#include "SwapChain.h"
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
        void createCommandBuffers();
        void createPipeLineLayout();
        void drawFrame();
        Window m_window{800, 600, "vulkan_window"};
        Device m_device{m_window};
        SwapChain m_swapChain{m_device, m_window.getExtent()};
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<Pipeline> m_pipeline;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::unique_ptr<Model> m_model;
    };

}  // namespace sge
