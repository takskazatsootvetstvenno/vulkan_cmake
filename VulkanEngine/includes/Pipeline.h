#pragma once
#include "Device.h"
#include "Shader.h"

#include <Descriptors.h>
#include "PipelineInputData.h"
#include "VulkanHelpUtils.h"

#include <string>
#include <vector>

namespace sge {
class Shader;

struct FixedPipelineStates  // TO DO add enums and abstractions
{
    bool depthTestEnable = true;
    bool depthWriteEnable = true;
    CompareOp depthOp = CompareOp::LESS;
    CullingMode cullingMode = CullingMode::BACK;
    FrontFace frontFace = FrontFace::COUNTER_CLOCKWISE;
};

class Pipeline {
 public:
    Pipeline(Device& device, const PipelineInputData& pipelineData);
    Pipeline(Device& device, PipelineInputData&& pipelineData);
    Pipeline(const Pipeline&) = delete;
    Pipeline& operator=(const Pipeline&) = delete;
    Pipeline& operator=(Pipeline&& other) = delete;
    Pipeline(Pipeline&& other) noexcept;
    ~Pipeline();

    const Shader& getShader() noexcept;
    const std::string& getName() const noexcept;
    void crateGraphicsPipeline();
    void bind(VkCommandBuffer commandBuffer) const noexcept;
    //static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height, FixedPipelineStates states);
    static std::vector<VkPipelineColorBlendAttachmentState> createDefaultColorAttachments();
    static const VkPipelineLayout createPipeLineLayout(const VkDevice device, VkDescriptorSetLayout setLayout);
    bool recreatePipelineShaders(const VkRenderPass renderPass);

 private:
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
    Device& m_device;
    VkPipeline m_graphicsPipeline;
    PipelineInputData m_pipelineData;
};
}  // namespace sge
