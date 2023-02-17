#include "ResourceSystem.h"
#include <algorithm>

namespace sge {

/*static*/ ResourceSystem& ResourceSystem::Instance() noexcept {
    static ResourceSystem resourceSystem;
    return resourceSystem;
}

void ResourceSystem::init(const Device* device) noexcept 
{ 
    m_device = device; 
    m_isInitilized = true;
}

uint32_t ResourceSystem::addFramebuffer(FrameBuffer&& framebuffer) {
    assert(framebuffer.valid() && "FrameBuffer is not valid!");
    m_frameBuffers.emplace_back(std::move(framebuffer));

    return static_cast<uint32_t>(m_frameBuffers.size() - 1);
}
bool FrameBuffer::valid() const noexcept {
    return m_isCreated;
}

const FrameBufferAttachment FrameBuffer::getFrameBufferAttachmentByID(const uint32_t id) const noexcept {
    if (!m_isCreated) {
        LOG_ERROR("ResourceSystem:FrameBuffer getFrameBufferAttachmentByID:"
                  << "\nError: "
                  << "FrameBuffer must be created before use! ");
        assert(false);
    }
    return m_attachments[id];
}

const VkFramebuffer FrameBuffer::getFrameBuffer() const noexcept { 
    return m_frameBuffer;
}

void FrameBuffer::create() { 
    auto count = std::count_if(m_attachments.begin(), m_attachments.end(), [](const FrameBufferAttachment att) {
        return att.layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
    });

    if (count > 1) {
        LOG_ERROR("RenderSystem::FrameBuffer:create: Wrong depth attachments. Render support only one depth/stencil attachment!");
        assert(false);
    }

    if (m_attachments.empty()) {
        LOG_ERROR(
            "RenderSystem::FrameBuffer:create: attachment list is empty! Can't create framebuffer and renderpass");
        assert(false);
    }
    std::vector<VkAttachmentReference> colorReferences;
    VkAttachmentReference depthReference = {};

    for (unsigned int i = 0; i < m_attachments.size(); ++i) {
        if (m_attachments[i].layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL)
            depthReference = {i, m_attachments[i].layout};
        else 
            colorReferences.push_back({i, m_attachments[i].layout});
    }

    VkSubpassDescription subpass = {};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.pColorAttachments = colorReferences.data();
    subpass.colorAttachmentCount = static_cast<uint32_t>(colorReferences.size());
    if (depthReference.layout == VK_IMAGE_LAYOUT_UNDEFINED) subpass.pDepthStencilAttachment = nullptr;
    else subpass.pDepthStencilAttachment = &depthReference;
    
    #if 1
    std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    #else
     std::array<VkSubpassDependency, 2> dependencies;

    dependencies[0].srcSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[0].dstSubpass = 0;
    dependencies[0].srcStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[0].dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[0].srcAccessMask = VK_ACCESS_SHADER_READ_BIT;
    dependencies[0].dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;

    dependencies[0].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;

    dependencies[1].srcSubpass = 0;
    dependencies[1].dstSubpass = VK_SUBPASS_EXTERNAL;
    dependencies[1].srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
    dependencies[1].dstStageMask = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
    dependencies[1].srcAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
    dependencies[1].dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

    dependencies[1].dependencyFlags = VK_DEPENDENCY_BY_REGION_BIT;
    #endif
    std::vector<VkAttachmentDescription> attchmentDescription;
    std::vector<VkImageView> imageViewAttachments;
    for (auto attachment : m_attachments) { 
        attchmentDescription.push_back(attachment.description); 
        imageViewAttachments.push_back(attachment.view);
    }

    VkRenderPassCreateInfo renderPassInfo = {};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.pAttachments = attchmentDescription.data();
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attchmentDescription.size());
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 2;
    renderPassInfo.pDependencies = dependencies.data();
    VK_CHECK_RESULT(vkCreateRenderPass(m_device.device(), &renderPassInfo, nullptr, &m_renderPass), "RenderSystem::FrameBuffer:create: Failed to create renderpass!");

    VkFramebufferCreateInfo frameBufferInfo{};
    frameBufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    frameBufferInfo.renderPass = m_renderPass;
    frameBufferInfo.pAttachments = imageViewAttachments.data();
    frameBufferInfo.attachmentCount = static_cast<uint32_t>(imageViewAttachments.size());
    frameBufferInfo.width = m_width;
    frameBufferInfo.height = m_height;
    frameBufferInfo.layers = 1;
    VK_CHECK_RESULT(vkCreateFramebuffer(m_device.device(), &frameBufferInfo, nullptr, &m_frameBuffer),
                    "RenderSystem::FrameBuffer:create: Failed to create framebuffer!");
    m_isCreated = true;
}

uint32_t ResourceSystem::addConstantBuffer(std::unique_ptr<Buffer>&& constantBuffer) {
    m_constBuffers.emplace_back(std::move(constantBuffer));
    return static_cast<uint32_t>(m_constBuffers.size() - 1);
}

void WorkFlow::setFrameVertexBuffer(const uint32_t frameID, const uint32_t vertexBufferID) noexcept {
    assert(frameID >= m_frames.size());
    m_frames[frameID].vertexBufferID = vertexBufferID;
}

void WorkFlow::setFrameIndexBuffer(const uint32_t frameID, const uint32_t indexBufferID) noexcept {
    assert(frameID >= m_frames.size());
    m_frames[frameID].indexBufferID = indexBufferID;
}

uint32_t ResourceSystem::addWorkFlow(WorkFlow&& workflow) { 
    m_workFlows.emplace_back(workflow);
    return static_cast<uint32_t>(m_workFlows.size() - 1);
}

const FrameBuffer& ResourceSystem::getFrameBufferByID(uint32_t id) noexcept {
    assert(!(id >= m_frameBuffers.size()));
    return m_frameBuffers[id];
}

const WorkFlow& ResourceSystem::getWorkFlow(uint32_t id) noexcept { 
    assert(!(id >= m_workFlows.size()));
    return m_workFlows[id]; 
}

const Buffer& ResourceSystem::getConstantBuffer(uint32_t id) noexcept {
    assert(!(id >= m_constBuffers.size()));
    return *(m_constBuffers)[id];
}
const DescriptorSetAttachment& ResourceSystem::getDescriptor(uint32_t id) noexcept {
    assert(!(id >= m_descriptors.size()));
    return m_descriptors[id];
}

const PipelineDescription& ResourceSystem::getPipeline(uint32_t id) noexcept {
    assert(!(id >= m_pipelines.size()));
    return m_pipelines[id];
}

uint32_t ResourceSystem::addDescriptor(DescriptorSetAttachment&& decriptorInfo) { 
    m_descriptors.push_back(std::move(decriptorInfo));
    return static_cast<uint32_t>(m_descriptors.size() - 1);
}

uint32_t ResourceSystem::addPipeline(PipelineDescription&& pipelineDescription) { 
    m_pipelines.emplace_back(std::move(pipelineDescription));
    return static_cast<uint32_t>(m_pipelines.size() - 1);
}

const VkRenderPass FrameBuffer::getRenderPass() const noexcept { return m_renderPass; }
void FrameBuffer::createAttachment(const VkFormat format, const VkImageUsageFlagBits usage,
                                   const bool isDepthStencil) noexcept {
    VkImageAspectFlags aspectMask = 0;

    if (usage & VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT) { aspectMask = VK_IMAGE_ASPECT_COLOR_BIT; }
    if (usage & VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT) {
        aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;  //| VK_IMAGE_ASPECT_STENCIL_BIT;
    }

    if(!(aspectMask > 0))
        LOG_ERROR("ResourceSystem: createAttachment:" << "\nError: " << "\"aspectMask\" should be > 0");

    VkImageCreateInfo imageInfo{};
    imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
    imageInfo.imageType = VK_IMAGE_TYPE_2D;
    imageInfo.format = format;
    imageInfo.extent.width = m_width;
    imageInfo.extent.height = m_height;
    imageInfo.extent.depth = 1;
    imageInfo.mipLevels = 1;
    imageInfo.arrayLayers = 1;
    imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
    imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
    imageInfo.usage = usage | VK_IMAGE_USAGE_SAMPLED_BIT;

    VkMemoryAllocateInfo memAlloc{};
    memAlloc.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
    VkMemoryRequirements memReqs{};

    FrameBufferAttachment newAttachment{};
    newAttachment.format = format;

    VK_CHECK_RESULT(vkCreateImage(m_device.device(), &imageInfo, nullptr, &newAttachment.image),
                    "ResourceSystem: createAttachment: Can't create image!");
    vkGetImageMemoryRequirements(m_device.device(), newAttachment.image, &memReqs);
    memAlloc.allocationSize = memReqs.size;
    memAlloc.memoryTypeIndex = m_device.findMemoryType(memReqs.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
   
    VK_CHECK_RESULT(vkAllocateMemory(m_device.device(), &memAlloc, nullptr, &newAttachment.mem), "ResourceSystem: createAttachment: failed to allocate memory");
    VK_CHECK_RESULT(vkBindImageMemory(m_device.device(), newAttachment.image, newAttachment.mem, 0),
                    "ResourceSystem: createAttachment: failed to bind image memory");

    VkImageViewCreateInfo imageView{};
    imageView.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
    imageView.viewType = VK_IMAGE_VIEW_TYPE_2D;
    imageView.format = format;
    imageView.subresourceRange = {};
    imageView.subresourceRange.aspectMask = aspectMask;
    imageView.subresourceRange.baseMipLevel = 0;
    imageView.subresourceRange.levelCount = 1;
    imageView.subresourceRange.baseArrayLayer = 0;
    imageView.subresourceRange.layerCount = 1;
    imageView.image = newAttachment.image;

    VK_CHECK_RESULT(vkCreateImageView(m_device.device(), &imageView, nullptr, &newAttachment.view), "ResourceSystem: createAttachment: failed to create ImageView!");

   newAttachment.description.format = format;
   newAttachment.description.samples = VK_SAMPLE_COUNT_1_BIT;
   newAttachment.description.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
   newAttachment.description.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
   newAttachment.description.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
   newAttachment.description.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
   newAttachment.description.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;

   
   if (isDepthStencil) {
        newAttachment.description.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL;
        newAttachment.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;
   } else {
        newAttachment.description.finalLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        newAttachment.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
   }
   m_attachments.emplace_back(newAttachment);
}

FrameBuffer::FrameBuffer(const Device& device, const uint32_t width, const uint32_t height) 
    : m_device(device), m_width(width), m_height(height) {}


const std::vector<WorkFlow::WorkFlowFrame>& WorkFlow::getFramesData() const noexcept {
    return m_frames;
  }

uint32_t WorkFlow::addNextFrame(uint32_t pipelineID, uint32_t vertexBufferID, uint32_t indexBufferID,
    bool hasVertexBuffer, bool hasIndexBuffer) {
   assert(!(hasIndexBuffer == true && hasVertexBuffer == false));
   m_frames.emplace_back(pipelineID, vertexBufferID, indexBufferID, hasIndexBuffer);
   return m_frames.size() - 1;
}

WorkFlow::WorkFlow(const std::string_view name):m_name(name) {}

}  // namespace sge
