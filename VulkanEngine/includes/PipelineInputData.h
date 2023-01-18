#pragma once

#include "Shader.h"
#include "VulkanHelpUtils.h"

#include <array>
#include <cassert>
#include <vector>


namespace sge {
class Pipeline;

class PipelineInputData final {
 public:
    class VertexData final {
        friend class sge::Pipeline;
     public:
        VertexData(const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
                   const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions);
        VertexData(std::vector<VkVertexInputBindingDescription>&& bindingDescriptions,
                   std::vector<VkVertexInputAttributeDescription>&& attributeDescriptions) noexcept;

     private:
        std::vector<VkVertexInputBindingDescription> m_bindingsDescriptions;
        std::vector<VkVertexInputAttributeDescription> m_attributeDescriptions;
    };

    class ColorBlendData final {
        friend class sge::Pipeline;
     public:
        ColorBlendData(const std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments);
        ColorBlendData(std::vector<VkPipelineColorBlendAttachmentState>&& colorBlendAttachments) noexcept;
        void setBlendConstants(const std::array<float, 4>& blendConstants) noexcept;
        void setLogicData(VkBool32 logicOpEnable, VkLogicOp logicOp) noexcept;

     private:
        std::vector<VkPipelineColorBlendAttachmentState> m_colorBlendAttachments;
        VkBool32 m_logicOpEnable = VK_FALSE;
        VkLogicOp m_logicOp = VK_LOGIC_OP_COPY;
        std::array<float, 4> m_blendConstants{};
    };

    class FixedFunctionsStages final {
        friend class sge::Pipeline;
     public:
        FixedFunctionsStages(const float width, const float height);

        void setViewportData(float x, float y, float width, float height, float minDepth, float maxDepth) noexcept;
        void setScissorData(int32_t x, int32_t y, uint32_t width, uint32_t height) noexcept;
        void setTopology(VkPrimitiveTopology topology) noexcept;
        void setPolygonMode(VkPolygonMode polygonMode) noexcept;
        void setCullingData(VkCullModeFlags cullMode, VkFrontFace frontFace) noexcept;
        void setDepthData(VkBool32 depthTestEnable, VkCompareOp depthCompareOp, VkBool32 depthWriteEnable,
                          VkBool32 boundsTestEnable, float minDepthBounds, float maxDepthBounds) noexcept;

     private:
        float m_width, m_height;
        /*inputAssembly*/
        VkPrimitiveTopology m_topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        VkBool32 m_primitiveRestartEnable = VK_FALSE;

        /*viewport*/
        VkViewport m_viewport;

        /*scissor*/
        VkRect2D m_scissor{};

        /*rasterization*/
        VkBool32 m_depthClampEnable = VK_FALSE;
        VkBool32 m_rasterizerDiscardEnable = VK_FALSE;
        VkPolygonMode m_polygonMode = VK_POLYGON_MODE_FILL;
        float m_lineWidth = 1.f;
        VkCullModeFlags m_cullMode = VK_CULL_MODE_BACK_BIT;
        VkFrontFace m_frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        VkBool32 m_depthBiasEnable = VK_FALSE;
        float m_depthBiasConstantFactor = 0.f;
        float m_depthBiasClamp = 0.f;
        float m_depthBiasSlopeFactor = 0.f;

        /*multisample*/
        VkBool32 m_sampleShadingEnable = VK_FALSE;
        VkSampleCountFlagBits m_rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        float m_minSampleShading = 1.f;
        const VkSampleMask* m_pSampleMask = nullptr;
        VkBool32 m_alphaToCoverageEnable = VK_FALSE;
        VkBool32 m_alphaToOneEnable = VK_FALSE;

        /*DepthStencil*/
        VkBool32 m_depthTestEnable = VK_TRUE;
        VkBool32 m_depthWriteEnable = VK_TRUE;
        VkCompareOp m_depthCompareOp = VK_COMPARE_OP_LESS;
        VkBool32 m_depthBoundsTestEnable = VK_FALSE;
        float m_minDepthBounds = 0.f;
        float m_maxDepthBounds = 1.f;
        VkBool32 m_stencilTestEnable = VK_FALSE;
        VkStencilOpState m_stencilFront = {};
        VkStencilOpState m_stencilBack = {};
    };

    PipelineInputData(VertexData&& vertexData, Shader&& shader, ColorBlendData&& colorBlendData,
                      VkPipelineLayout pipelineLayout, FixedFunctionsStages&& fixedFunctionStage,
                      VkRenderPass renderPass)
        : m_vertexData(std::move(vertexData)), m_shader(std::move(shader)),
          m_fixedFunctionStage(std::move(fixedFunctionStage)), m_colorBlendData(std::move(colorBlendData)),
          m_pipelineLayout(pipelineLayout), m_renderPass(renderPass){};

    PipelineInputData(const VertexData& vertexData, const Shader& shader, const ColorBlendData& colorBlendData,
                      VkPipelineLayout pipelineLayout, const FixedFunctionsStages& fixedFunctionStage,
                      VkRenderPass renderPass)
        : m_vertexData(vertexData), m_shader(shader), m_fixedFunctionStage(fixedFunctionStage),
          m_colorBlendData(colorBlendData), m_pipelineLayout(pipelineLayout), m_renderPass(renderPass){};

    const FixedFunctionsStages& getFixedFunctionsStages() { return m_fixedFunctionStage; }
    const VertexData& getVertexData() { return m_vertexData; }
    Shader& getShader() { return m_shader; }
    const ColorBlendData& getColorBlendData() { return m_colorBlendData; }
    const VkPipelineLayout getPipelineLayout() { return m_pipelineLayout; }
    const VkRenderPass getRenderPass() { return m_renderPass; }
    const uint32_t getSubpass() { return m_subpass; }

 private:
    VertexData m_vertexData;
    Shader m_shader;
    FixedFunctionsStages m_fixedFunctionStage;
    ColorBlendData m_colorBlendData;
    VkPipelineLayout m_pipelineLayout;
    VkRenderPass m_renderPass;
    uint32_t m_subpass = 0;
};
}  // namespace sge
