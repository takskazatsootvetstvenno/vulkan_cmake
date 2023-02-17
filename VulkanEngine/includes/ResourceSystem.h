#include "Pipeline.h"
#include "Device.h"
#include "Buffer.h"

#include <vector>
namespace sge {
struct FrameBufferAttachment {
    VkImage image{};
    VkDeviceMemory mem{};
    VkImageView view{};
    VkFormat format{};
    VkAttachmentDescription description{};
    VkImageLayout layout;
};

struct DescriptorSetAttachment {
    std::unique_ptr<DescriptorSetLayout> layout;
    VkDescriptorSet set;
};

struct PipelineDescription {
    std::string name;
    VkPipelineLayout pipelineLayout;
    Pipeline pipeline;
    uint32_t descriptorID;
    uint32_t framebufferID;
};

class WorkFlow 
{
 public:
    struct WorkFlowFrame {
        uint32_t pipelineID = 0;
        uint32_t vertexBufferID = 0;
        uint32_t indexBufferID = 0;
        bool hasVertexBuffer = false;
        bool hasIndexBuffer = false;
    };

    WorkFlow(const std::string_view name);
    uint32_t addNextFrame(uint32_t pipelineID, uint32_t vertexBufferID, uint32_t indexBufferID, bool hasVertexBuffer,
                          bool hasIndexBuffer);
    const std::vector<WorkFlow::WorkFlowFrame>& getFramesData() const noexcept;
    void setFrameVertexBuffer(const uint32_t frameID, const uint32_t vertexBufferID) noexcept;
    void setFrameIndexBuffer(const uint32_t frameID, const uint32_t indexBufferID) noexcept;

 private:
    std::string m_name;
    std::vector<WorkFlowFrame> m_frames;
};

class FrameBuffer {
 public:
    FrameBuffer(const Device& device, const uint32_t width, const uint32_t height);
    void createAttachment(const VkFormat format, const VkImageUsageFlagBits usage, const bool isDepthStencil = false) noexcept;

    // Func returns ID in frameBuffer vector
    void create();
    bool valid() const noexcept;
    const VkRenderPass getRenderPass() const noexcept;
    const VkFramebuffer getFrameBuffer() const noexcept;
    const FrameBufferAttachment getFrameBufferAttachmentByID(const uint32_t id) const noexcept;
 private:
    uint32_t m_width, m_height;
    VkFramebuffer m_frameBuffer = nullptr;
    VkRenderPass m_renderPass = nullptr;
    std::vector<FrameBufferAttachment> m_attachments;
    std::string m_frameBufferName;
    const Device& m_device;
    bool m_isCreated = false;
};

class ResourceSystem {
 public:
    static ResourceSystem& Instance() noexcept;
    void init(const Device* device) noexcept;
    ResourceSystem(const ResourceSystem&) = delete;
    ResourceSystem(ResourceSystem&&) = delete;
    ResourceSystem& operator=(const ResourceSystem&) = delete;
    ResourceSystem& operator=(ResourceSystem&&) = delete;

    uint32_t addFramebuffer(FrameBuffer&& framebuffer);
    uint32_t addDescriptor(DescriptorSetAttachment&& decriptorInfo);
    uint32_t addPipeline(PipelineDescription&& pipelineDescription);
    uint32_t addConstantBuffer(std::unique_ptr<Buffer>&& buffer);
    uint32_t addWorkFlow(WorkFlow&& workflow);

    const FrameBuffer& getFrameBufferByID(uint32_t id) noexcept;
    const DescriptorSetAttachment& getDescriptor(uint32_t id) noexcept;
    const PipelineDescription& getPipeline(uint32_t id) noexcept;
    const Buffer& getConstantBuffer(uint32_t id) noexcept;
    const WorkFlow& getWorkFlow(uint32_t id) noexcept;
    
 private:
    const Device* m_device = nullptr;
    std::vector<FrameBuffer> m_frameBuffers;
    std::vector<DescriptorSetAttachment> m_descriptors;
    std::vector<PipelineDescription> m_pipelines;
    std::vector<std::unique_ptr<Buffer>> m_constBuffers;
    std::vector<WorkFlow> m_workFlows;
    bool m_isInitilized = false;
    ~ResourceSystem(){};
    ResourceSystem(){};
};
}