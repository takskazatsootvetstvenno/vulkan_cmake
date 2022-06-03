 #pragma once
#include "Device.h"
#include "Shader.h"
#include <Descriptors.h>
#include <string>
namespace sge {
    class Shader;
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
        Pipeline(Device& device, Shader&& shader, const PipelineConfigInfo& configInfo);
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;
        ~Pipeline();

        const Shader& getShader() const noexcept;
        void crateGraphicsPipeline(const PipelineConfigInfo& configInfo);
        void bind(VkCommandBuffer commandBuffer) const noexcept;
        const DescriptorSetLayout& getDescriptorLayout() noexcept;
        static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height);

    private:
        VkShaderModule createShaderModule(const std::string& code);
        Device& m_device;
        std::unique_ptr<DescriptorSetLayout> m_descriptorLayout;
        Shader m_shader;
        VkPipeline m_graphicsPipeline;
	};
}
