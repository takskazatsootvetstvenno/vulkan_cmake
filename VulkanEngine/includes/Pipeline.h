 #pragma once
#include "Device.h"
#include <string>
namespace sge {
	struct PipelineConfigInfo {
		PipelineConfigInfo() = default;
        PipelineConfigInfo(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo& operator=(const PipelineConfigInfo&) = delete;
        PipelineConfigInfo(PipelineConfigInfo&&) = default;
        PipelineConfigInfo& operator=(PipelineConfigInfo&&) = default;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;
        //VkPipelineDynamicStateCreateInfo dynamicStateInfo;
        //std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        
		VkViewport viewport;
		VkRect2D scissor;
        uint32_t subpass = 0;
	};

	class Pipeline
	{
    public:
        Pipeline(Device& device, const std::string_view vertFilePath, const std::string_view fragFilePath, const PipelineConfigInfo& configInfo);
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;
        ~Pipeline();

        std::string readFile(const std::string_view filepath);
        void crateGraphicsPipeline(const std::string_view vertFilePath, const std::string_view fragFilePath, const PipelineConfigInfo& configInfo);
        void bind(VkCommandBuffer commandBuffer) const noexcept;
        static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height);
        VkShaderModule createShaderModule(const std::string& code);
    private:
        Device& m_device;
        VkPipeline m_graphicsPipeline;
	};
}
