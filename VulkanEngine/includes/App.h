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
        void freeCommandBuffers();
        void createPipeLineLayout();
        void drawFrame() noexcept;
        void recreateSwapChain() noexcept;
        void recordCommandBuffer(const int imageIndex);

        Window m_window{800, 600, "vulkan_window"};
        Device m_device{m_window};
        std::unique_ptr<SwapChain> m_swapChain;
        VkPipelineLayout m_pipelineLayout;
        std::unique_ptr<Pipeline> m_pipeline;
        std::vector<VkCommandBuffer> m_commandBuffers;
        std::unique_ptr<Model> m_model;
    };

} 
