#include "Pipeline.h"

#include "Logger.h"
#include "Model.h"
#include "VulkanHelpUtils.h"

#include <cassert>

namespace sge {
/*
constexpr VkCompareOp toVulkanCompareOp(const CompareOp op) noexcept {
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
    ;
}
constexpr VkCullModeFlagBits toVulkanCullingMode(const CullingMode mode) noexcept {
    switch (mode) {
        case sge::CullingMode::NONE: return VkCullModeFlagBits::VK_CULL_MODE_NONE;
        case sge::CullingMode::FRONT: return VkCullModeFlagBits::VK_CULL_MODE_FRONT_BIT;
        case sge::CullingMode::BACK: return VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT;
        case sge::CullingMode::FRONT_AND_BACK: return VkCullModeFlagBits::VK_CULL_MODE_FRONT_AND_BACK;
    }
    return VkCullModeFlagBits::VK_CULL_MODE_NONE;
}
constexpr VkFrontFace toVulkanFrontFace(const FrontFace face) noexcept {
    switch (face) {
        case sge::FrontFace::COUNTER_CLOCKWISE: return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
        case sge::FrontFace::CLOCKWISE: return VkFrontFace::VK_FRONT_FACE_CLOCKWISE;
    }
    return VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE;
}
*/
const Shader& Pipeline::getShader() noexcept { return m_pipelineData.getShader(); }

void Pipeline::crateGraphicsPipeline() {
    assert(m_pipelineData.getPipelineLayout() != VK_NULL_HANDLE &&
           "Cannot create graphics pipeline: no pipelineLauout provided");
    assert(m_pipelineData.getRenderPass() != VK_NULL_HANDLE &&
           "Cannot create graphics pipeline: no renderPass provided");

    const auto& vertCode = m_pipelineData.getShader().getVertexShader();
    const auto& fragCode = m_pipelineData.getShader().getFragmentShader();
    const auto& geometryCode = m_pipelineData.getShader().isGeometryShaderPresent() ? getShader().getGeometryShader() :
                                                                                      std::vector<uint32_t>();

    VkShaderModule vertShaderModule = createShaderModule(vertCode);
    VkShaderModule fragShaderModule = createShaderModule(fragCode);
    VkShaderModule geomShaderModule = m_pipelineData.getShader().isGeometryShaderPresent() ?
                                          createShaderModule(geometryCode) :
                                          VkShaderModule{};

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
    vertShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    vertShaderStageInfo.stage = VK_SHADER_STAGE_VERTEX_BIT;
    vertShaderStageInfo.module = vertShaderModule;
    vertShaderStageInfo.pName = "main";
    vertShaderStageInfo.flags = 0;
    vertShaderStageInfo.pNext = nullptr;
    vertShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
    fragShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
    fragShaderStageInfo.stage = VK_SHADER_STAGE_FRAGMENT_BIT;
    fragShaderStageInfo.module = fragShaderModule;
    fragShaderStageInfo.pName = "main";
    fragShaderStageInfo.flags = 0;
    fragShaderStageInfo.pNext = nullptr;
    fragShaderStageInfo.pSpecializationInfo = nullptr;

    VkPipelineShaderStageCreateInfo geometryShaderStageInfo{};
    if (m_pipelineData.getShader().isGeometryShaderPresent()) {
        geometryShaderStageInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        geometryShaderStageInfo.stage = VK_SHADER_STAGE_GEOMETRY_BIT;
        geometryShaderStageInfo.module = geomShaderModule;
        geometryShaderStageInfo.pName = "main";
        geometryShaderStageInfo.flags = 0;
        geometryShaderStageInfo.pNext = nullptr;
        geometryShaderStageInfo.pSpecializationInfo = nullptr;
    }
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo, geometryShaderStageInfo};

    auto bindingsDescriptions = Vertex::getBindingDescription();
    auto attributeDescriptions = Vertex::getAttributeDescription();

    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingsDescriptions.size());
    vertexInputInfo.pVertexBindingDescriptions = bindingsDescriptions.data();
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineColorBlendStateCreateInfo colorBlendInfo{};
    colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
    colorBlendInfo.logicOpEnable = VK_FALSE;
    colorBlendInfo.logicOp = VK_LOGIC_OP_COPY;
    colorBlendInfo.attachmentCount = 1;   //1????
    colorBlendInfo.pAttachments = m_pipelineData.getColorBlendData().m_colorBlendAttachments.data();
    colorBlendInfo.blendConstants[0] = 0.f;
    colorBlendInfo.blendConstants[1] = 0.f;
    colorBlendInfo.blendConstants[2] = 0.f;
    colorBlendInfo.blendConstants[3] = 0.f;

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
    inputAssembly.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    inputAssembly.primitiveRestartEnable = m_pipelineData.getFixedFunctionsStages().m_primitiveRestartEnable;
    inputAssembly.topology = m_pipelineData.getFixedFunctionsStages().m_topology;

    VkPipelineViewportStateCreateInfo viewportStateInfo{};
    viewportStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
    viewportStateInfo.viewportCount = 1;
    viewportStateInfo.pViewports = &m_pipelineData.getFixedFunctionsStages().m_viewport;
    viewportStateInfo.scissorCount = 1;
    viewportStateInfo.pScissors = &m_pipelineData.getFixedFunctionsStages().m_scissor;

    VkPipelineRasterizationStateCreateInfo rasterizationInfo{};
    rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    rasterizationInfo.depthClampEnable = m_pipelineData.getFixedFunctionsStages().m_depthClampEnable;
    rasterizationInfo.rasterizerDiscardEnable = m_pipelineData.getFixedFunctionsStages().m_rasterizerDiscardEnable;
    rasterizationInfo.polygonMode = m_pipelineData.getFixedFunctionsStages().m_polygonMode;
    rasterizationInfo.lineWidth = m_pipelineData.getFixedFunctionsStages().m_lineWidth;
    rasterizationInfo.cullMode = m_pipelineData.getFixedFunctionsStages().m_cullMode;
    rasterizationInfo.frontFace = m_pipelineData.getFixedFunctionsStages().m_frontFace;
    rasterizationInfo.depthBiasEnable = m_pipelineData.getFixedFunctionsStages().m_depthBiasEnable;
    rasterizationInfo.depthBiasConstantFactor = m_pipelineData.getFixedFunctionsStages().m_depthBiasConstantFactor;
    rasterizationInfo.depthBiasClamp = m_pipelineData.getFixedFunctionsStages().m_depthBiasClamp;
    rasterizationInfo.depthBiasSlopeFactor = m_pipelineData.getFixedFunctionsStages().m_depthBiasSlopeFactor;

    VkPipelineMultisampleStateCreateInfo multisampleInfo{};
    multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    multisampleInfo.sampleShadingEnable = m_pipelineData.getFixedFunctionsStages().m_sampleShadingEnable;
    multisampleInfo.rasterizationSamples = m_pipelineData.getFixedFunctionsStages().m_rasterizationSamples;
    multisampleInfo.minSampleShading = m_pipelineData.getFixedFunctionsStages().m_minSampleShading;
    multisampleInfo.pSampleMask = m_pipelineData.getFixedFunctionsStages().m_pSampleMask;
    multisampleInfo.alphaToCoverageEnable = m_pipelineData.getFixedFunctionsStages().m_alphaToCoverageEnable;
    multisampleInfo.alphaToOneEnable = m_pipelineData.getFixedFunctionsStages().m_alphaToOneEnable;

    VkPipelineDepthStencilStateCreateInfo depthStencilInfo{};
    depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    depthStencilInfo.depthTestEnable = m_pipelineData.getFixedFunctionsStages().m_depthTestEnable;
    depthStencilInfo.depthWriteEnable = m_pipelineData.getFixedFunctionsStages().m_depthWriteEnable;
    depthStencilInfo.depthCompareOp = m_pipelineData.getFixedFunctionsStages().m_depthCompareOp;
    depthStencilInfo.depthBoundsTestEnable = m_pipelineData.getFixedFunctionsStages().m_depthBoundsTestEnable;
    depthStencilInfo.minDepthBounds = m_pipelineData.getFixedFunctionsStages().m_minDepthBounds;
    depthStencilInfo.maxDepthBounds = m_pipelineData.getFixedFunctionsStages().m_maxDepthBounds;
    depthStencilInfo.stencilTestEnable = m_pipelineData.getFixedFunctionsStages().m_stencilTestEnable;
    depthStencilInfo.front = m_pipelineData.getFixedFunctionsStages().m_stencilFront;
    depthStencilInfo.back = m_pipelineData.getFixedFunctionsStages().m_stencilBack;

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = m_pipelineData.getShader().isGeometryShaderPresent() ? 3 : 2;
    pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo;
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportStateInfo;
    pipelineInfo.pRasterizationState = &rasterizationInfo;
    pipelineInfo.pMultisampleState = &multisampleInfo;

    pipelineInfo.pColorBlendState = &colorBlendInfo;
    pipelineInfo.pDepthStencilState = &depthStencilInfo;
    pipelineInfo.pDynamicState = nullptr;

    pipelineInfo.layout = m_pipelineData.getPipelineLayout();
    pipelineInfo.renderPass = m_pipelineData.getRenderPass();
    pipelineInfo.subpass = m_pipelineData.getSubpass();  // 0

    pipelineInfo.basePipelineIndex = -1;
    pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;

    auto result = vkCreateGraphicsPipelines(m_device.device(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr,
                                            &m_graphicsPipeline);
    VK_CHECK_RESULT(result, "Failed to create graphics pipeline")
    vkDestroyShaderModule(m_device.device(), vertShaderModule, nullptr);
    vkDestroyShaderModule(m_device.device(), fragShaderModule, nullptr);
    if (m_pipelineData.getShader().isGeometryShaderPresent())
        vkDestroyShaderModule(m_device.device(), geomShaderModule, nullptr);
}

void Pipeline::bind(VkCommandBuffer commandBuffer) const noexcept {
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, m_graphicsPipeline);
}

/*
PipelineConfigInfo Pipeline::createDefaultPipeline(uint32_t width, uint32_t height, FixedPipelineStates states) {
    PipelineConfigInfo configInfo{};
    configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
    configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
    configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;

    configInfo.viewport.x = 0.f;
    configInfo.viewport.y = 0.f;
    configInfo.viewport.width = static_cast<float>(width);
    configInfo.viewport.height = static_cast<float>(height);
    configInfo.viewport.minDepth = 0.f;
    configInfo.viewport.maxDepth = 1.f;

    configInfo.scissor.offset = {0, 0};
    configInfo.scissor.extent = {width, height};

    configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
    configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
    configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
    configInfo.rasterizationInfo.lineWidth = 1.0f;
    configInfo.rasterizationInfo.cullMode = toVulkanCullingMode(states.cullingMode);
    configInfo.rasterizationInfo.frontFace = toVulkanFrontFace(states.frontFace);
    configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
    configInfo.rasterizationInfo.depthBiasConstantFactor = 0.f;
    configInfo.rasterizationInfo.depthBiasClamp = 0.f;
    configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.f;

    configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
    configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
    configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
    configInfo.multisampleInfo.minSampleShading = 1.0f;
    configInfo.multisampleInfo.pSampleMask = nullptr;
    configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE;
    configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;

    configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
    configInfo.depthStencilInfo.depthWriteEnable = states.depthWriteEnable;
    configInfo.depthStencilInfo.depthCompareOp = toVulkanCompareOp(states.depthOp);
    configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.minDepthBounds = 0.f;
    configInfo.depthStencilInfo.maxDepthBounds = 1.f;
    configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
    configInfo.depthStencilInfo.front = {};
    configInfo.depthStencilInfo.back = {};
    configInfo.userDefinedStates = states;
    return configInfo;
}
*/
std::vector<VkPipelineColorBlendAttachmentState> Pipeline::createDefaultColorAttachments() {
    std::vector<VkPipelineColorBlendAttachmentState> colorBlendAttachment(2);
    colorBlendAttachment[0].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment[0].blendEnable = VK_FALSE;
    colorBlendAttachment[0].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[0].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[0].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[0].alphaBlendOp = VK_BLEND_OP_ADD;

    colorBlendAttachment[1].colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                                             VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
    colorBlendAttachment[1].blendEnable = VK_FALSE;
    colorBlendAttachment[1].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[1].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[1].colorBlendOp = VK_BLEND_OP_ADD;
    colorBlendAttachment[1].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    colorBlendAttachment[1].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    colorBlendAttachment[1].alphaBlendOp = VK_BLEND_OP_ADD;
    return colorBlendAttachment;
}

const VkPipelineLayout Pipeline::createPipeLineLayout(const VkDevice device, VkDescriptorSetLayout setLayout) {
    VkPipelineLayout pipelineLayout;
    std::vector<VkDescriptorSetLayout> descriptorSetLayouts{setLayout};
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    pipelineLayoutInfo.setLayoutCount = static_cast<uint32_t>(descriptorSetLayouts.size());
    pipelineLayoutInfo.pSetLayouts = descriptorSetLayouts.data();
    pipelineLayoutInfo.pushConstantRangeCount = 0;
    pipelineLayoutInfo.pPushConstantRanges = nullptr;
    auto result = vkCreatePipelineLayout(device, &pipelineLayoutInfo, nullptr, &pipelineLayout);
    VK_CHECK_RESULT(result, "Failed to create pipeline layout");
    return pipelineLayout;
}

bool Pipeline::recreatePipelineShaders() {
    Shader newShader(m_pipelineData.getShader().getVertexShaderPath(),
                     m_pipelineData.getShader().getFragmentShaderPath(),
                     m_pipelineData.getShader().getGeometryShaderPath(), m_pipelineData.getShader().getDefines());
    if (newShader.isValid() == true) {
        m_pipelineData.getShader() = std::move(newShader);
        VK_CHECK_RESULT(vkDeviceWaitIdle(m_device.device()), "Failed to wait idle during recreationg pipeline shaders");
        vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr);
        crateGraphicsPipeline();
    } else {
        LOG_ERROR("Can't recreate shaders in pipeline! Used last suitable shader")
        return false;
    }
    return true;
}

VkShaderModule Pipeline::createShaderModule(const std::vector<uint32_t>& code) {
    VkShaderModuleCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
    createInfo.codeSize = code.size() * sizeof(uint32_t);
    createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());
    VkShaderModule shaderModule;
    auto result = vkCreateShaderModule(m_device.device(), &createInfo, nullptr, &shaderModule);
    VK_CHECK_RESULT(result, "Failed to create shader module!")
    return shaderModule;
}
Pipeline::Pipeline(Device& device, const PipelineInputData& pipelineData)
    : m_device(device), m_pipelineData(pipelineData) {
    crateGraphicsPipeline();
}
Pipeline::Pipeline(Device& device, PipelineInputData&& pipelineData) : m_device(device), m_pipelineData(std::move(pipelineData)) {
    crateGraphicsPipeline();
}
Pipeline::~Pipeline() { vkDestroyPipeline(m_device.device(), m_graphicsPipeline, nullptr); }
}  // namespace sge
