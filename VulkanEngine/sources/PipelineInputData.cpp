#include "PipelineInputData.h"

namespace sge {
PipelineInputData::VertexData::VertexData(
    const std::vector<VkVertexInputBindingDescription>& bindingDescriptions,
    const std::vector<VkVertexInputAttributeDescription>& attributeDescriptions)
    : m_bindingsDescriptions(bindingDescriptions), m_attributeDescriptions(attributeDescriptions) {}

PipelineInputData::VertexData::VertexData(
    std::vector<VkVertexInputBindingDescription>&& bindingDescriptions,
    std::vector<VkVertexInputAttributeDescription>&& attributeDescriptions) noexcept
    : m_bindingsDescriptions(std::move(bindingDescriptions)),
      m_attributeDescriptions(std::move(attributeDescriptions)) {}

PipelineInputData::FixedFunctionsStages::FixedFunctionsStages(const float width, const float height)
    : m_width(width),
      m_height(height),
      m_viewport{0.f, 0.f, width, height, 0.f, 1.f},
      m_scissor{{0, 0},
      {static_cast<uint32_t>(width), static_cast<uint32_t>(height)}} {}

void PipelineInputData::FixedFunctionsStages::setViewportData(float x, float y, float width, float height,
                                                              float minDepth = 0.f, float maxDepth = 1.f) noexcept {
    m_viewport = {x, y, width, height, minDepth, maxDepth};
}

void PipelineInputData::FixedFunctionsStages::setScissorData(int32_t x, int32_t y, uint32_t width,
                                                             uint32_t height) noexcept {
    m_scissor = {x, y, width, height};
}

void PipelineInputData::FixedFunctionsStages::setTopology(VkPrimitiveTopology topology) noexcept {
    m_topology = topology;
}

void PipelineInputData::FixedFunctionsStages::setPolygonMode(VkPolygonMode polygonMode) noexcept {
    m_polygonMode = polygonMode;
}

void PipelineInputData::FixedFunctionsStages::setCullingData(VkCullModeFlags cullMode, VkFrontFace frontFace) noexcept {
    m_cullMode = cullMode;
    m_frontFace = frontFace;
}

void PipelineInputData::FixedFunctionsStages::setDepthData(VkBool32 depthTestEnable, VkCompareOp depthCompareOp,
                                                           VkBool32 depthWriteEnable = VK_TRUE,
                                                           VkBool32 boundsTestEnable = VK_FALSE,
                                                           float minDepthBounds = 0.f,
                                                           float maxDepthBounds = 1.f) noexcept {
    m_depthTestEnable = depthTestEnable;
    m_depthCompareOp = depthCompareOp;
    m_depthWriteEnable = depthWriteEnable;
    m_depthBoundsTestEnable = boundsTestEnable;
    m_minDepthBounds = minDepthBounds;
    m_maxDepthBounds = maxDepthBounds;
}

PipelineInputData::ColorBlendData::ColorBlendData(
    const std::vector<VkPipelineColorBlendAttachmentState>& colorBlendAttachments)
    : m_colorBlendAttachments(colorBlendAttachments) {}

PipelineInputData::ColorBlendData::ColorBlendData(
    std::vector<VkPipelineColorBlendAttachmentState>&& colorBlendAttachments) noexcept
    : m_colorBlendAttachments(std::move(colorBlendAttachments)) {}

void PipelineInputData::ColorBlendData::setBlendConstants(const std::array<float, 4>& blendConstants) noexcept {
    m_blendConstants = blendConstants;
}
void PipelineInputData::ColorBlendData::setLogicData(VkBool32 logicOpEnable, VkLogicOp logicOp) noexcept {
    m_logicOpEnable = logicOpEnable;
    m_logicOp = logicOp;
}
}  // namespace sge
