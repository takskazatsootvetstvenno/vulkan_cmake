#pragma once
#include "Device.h"
#include "Shader.h"

#include <Descriptors.h>
#include "PipelineInputData.h"

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

enum class CullingMode { NONE = 0, FRONT = 0x00000001, BACK = 0x00000002, FRONT_AND_BACK = 0x00000003 };

enum class FrontFace { COUNTER_CLOCKWISE = 0, CLOCKWISE = 1 };
struct FixedPipelineStates  // TO DO add enums and abstractions
{
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
    Pipeline(Pipeline&&) = delete;
    Pipeline& operator=(Pipeline&&) = delete;
    ~Pipeline();

    const Shader& getShader() noexcept;
    void crateGraphicsPipeline();
    void bind(VkCommandBuffer commandBuffer) const noexcept;
    //static PipelineConfigInfo createDefaultPipeline(uint32_t width, uint32_t height, FixedPipelineStates states);
    static std::vector<VkPipelineColorBlendAttachmentState> createDefaultColorAttachments();
    static const VkPipelineLayout createPipeLineLayout(const VkDevice device, VkDescriptorSetLayout setLayout);
    bool recreatePipelineShaders();

 private:
    VkShaderModule createShaderModule(const std::vector<uint32_t>& code);
    Device& m_device;
    VkPipeline m_graphicsPipeline;
    PipelineInputData m_pipelineData;
};
}  // namespace sge
