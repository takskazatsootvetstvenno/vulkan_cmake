 #pragma once
#include "Device.h"
#include "Shader.h"
#include <Descriptors.h>
#include <string>
#include <vector>

namespace sge {
    class Shader;
    enum class CompareOp {
        NEVER = 0,
        LESS = 1,
        EQUAL = 2,
        LESS_OR_EQUAL = 3,
        GREATER = 4,
        NOT_EQUAL = 5,
        GREATER_OR_EQUAL = 6,
        ALWAYS = 7
    };

    enum class CullingMode {
        NONE = 0,
        FRONT = 0x00000001,
        BACK = 0x00000002,
        FRONT_AND_BACK = 0x00000003
    };

    enum class FrontFace {
        COUNTER_CLOCKWISE = 0,
        CLOCKWISE = 1
    };
    struct FixedPipelineStates //TO DO add enums and abstractions
    {
        bool depthWriteEnable = true;
        CompareOp depthOp = CompareOp::LESS;
        CullingMode cullingMode = CullingMode::BACK;
        FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE;
    };
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
        FixedPipelineStates userDefinedStates;
	};
 
	class Pipeline
	{
    public:
        Pipeline(Device& device, Shader&& shader, PipelineConfigInfo&& configInfo);
        Pipeline(const Pipeline&) = delete;
        Pipeline& operator=(const Pipeline&) = delete;
        Pipeline(Pipeline&&) = delete;
        Pipeline& operator=(Pipeline&&) = delete;
        ~Pipeline();

        const Shader& getShader() const noexcept;
        void crateGraphicsPipeline(const PipelineConfigInfo& configInfo);
        void bind(VkCommandBuffer commandBuffer) const noexcept;
        FixedPipelineStates getPipelineStates() const noexcept;
        static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height, FixedPipelineStates states);

    private:
        VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
        PipelineConfigInfo m_pipelineInfo;
        Device& m_device;
        Shader m_shader;
        VkPipeline m_graphicsPipeline;
	};
}
