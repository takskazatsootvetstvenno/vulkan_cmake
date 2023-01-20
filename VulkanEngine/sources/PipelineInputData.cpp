#include "PipelineInputData.h"

namespace sge {

constexpr VkCompareOp toVulkanType(const CompareOp op) noexcept {
    switch (op) {
        case CompareOp::NEVER: return VkCompareOp::VK_COMPARE_OP_NEVER;
        case CompareOp::LESS: return VkCompareOp::VK_COMPARE_OP_LESS;
        case CompareOp::EQUAL: return VkCompareOp::VK_COMPARE_OP_EQUAL;
        case CompareOp::LESS_OR_EQUAL: return VkCompareOp::VK_COMPARE_OP_LESS_OR_EQUAL;
        case CompareOp::GREATER: return VkCompareOp::VK_COMPARE_OP_GREATER;
        case CompareOp::NOT_EQUAL: return VkCompareOp::VK_COMPARE_OP_NOT_EQUAL;
        case CompareOp::GREATER_OR_EQUAL: return VkCompareOp::VK_COMPARE_OP_GREATER_OR_EQUAL;
        case CompareOp::ALWAYS: return VkCompareOp::VK_COMPARE_OP_ALWAYS;
    }
    return VkCompareOp::VK_COMPARE_OP_NEVER;
}

constexpr VkCullModeFlagBits toVulkanType(const CullingMode mode) noexcept {
    switch (mode) {
        case sge::CullingMode::NONE: return VkCullModeFlagBits::VK_CULL_MODE_NONE;
        case sge::CullingMode::FRONT: return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
        case sge::CullingMode::BACK: return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
        case sge::CullingMode::FRONT_AND_BACK: return VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK;
    }
    return VkCullModeFlagBits::VK_CULL_MODE_NONE;
}

constexpr VkFrontFace toVulkanType(const FrontFace face) noexcept {
    switch (face) {
        case sge::FrontFace::COUNTER_CLOCKWISE: return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case sge::FrontFace::CLOCKWISE: return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
    }
    return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
}

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

void PipelineInputData::FixedFunctionsStages::setCullingData(CullingMode cullMode, FrontFace frontFace) noexcept {
    m_cullMode = toVulkanType(cullMode);
    m_frontFace = toVulkanType(frontFace);
}

void PipelineInputData::FixedFunctionsStages::setDepthData(VkBool32 depthTestEnable, VkCompareOp depthCompareOp,
                                                           VkBool32 depthWriteEnable,
                                                           VkBool32 boundsTestEnable,
                                                           float minDepthBounds,
                                                           float maxDepthBounds) noexcept {
    m_depthTestEnable = depthTestEnable;
    m_depthCompareOp = depthCompareOp;
    m_depthWriteEnable = depthWriteEnable;
    m_depthBoundsTestEnable = boundsTestEnable;
    m_minDepthBounds = minDepthBounds;
    m_maxDepthBounds = maxDepthBounds;
}

void PipelineInputData::FixedFunctionsStages::setDepthData(bool depthTestEnable, CompareOp depthCompareOp,
                                                           bool depthWriteEnable,
                                                           bool boundsTestEnable,
                                                           float minDepthBounds,
                                                           float maxDepthBounds) noexcept {
    m_depthTestEnable = depthTestEnable;
    m_depthCompareOp = toVulkanType(depthCompareOp);
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
