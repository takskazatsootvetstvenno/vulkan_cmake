#include "Pipeline.h"

#include <vector>
namespace sge {

struct PipelineNode {
    Pipeline m_pipeline;
    std::vector<uint32_t> m_inputTexturesID;
    std::vector<uint32_t> m_inputCB;
    std::vector<uint32_t> m_outputAttachments;
    uint32_t m_framebufferID = -1;
    std::string m_pipelineName;
};

class RenderSystem {
 public:
    static RenderSystem& Instance() noexcept;

    RenderSystem(const RenderSystem&) = delete;
    RenderSystem(RenderSystem&&) = delete;
    RenderSystem& operator=(const RenderSystem&) = delete;
    RenderSystem& operator=(RenderSystem&&) = delete;

 private:
    std::vector<PipelineNode> m_pipelines;
    ~RenderSystem(){};
    RenderSystem(){};
};
}  // namespace sge
