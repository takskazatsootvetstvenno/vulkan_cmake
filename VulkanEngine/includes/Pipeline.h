 #pragma once
#include "Device.h"
#include <string>
namespace sge {
	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
		VkViewport viewport;
		VkRect2D scissor;
        uint32_t subpass = 0;
	};

	class Pipeline
	{
    public:
        Pipeline(Device& device, const std::string vertFilePath, const std::string fragFilePath, const PipelineConfigInfo& configInfo);
        std::string readFile(const std::string& filepath);
        void crateGraphicsPipeline(const std::string& vertFilePath, const std::string& fragFilePath, const PipelineConfigInfo& configInfo);

        void bind(VkCommandBuffer commandBuffer);

        static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height);
        VkShaderModule createShaderModule(const std::string& code);
        ~Pipeline();
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;
    private:
        Device& m_device;
        VkPipeline m_graphicsPipeline;
	};
}
