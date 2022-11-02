#pragma once
#include "Descriptors.h"
#include "Mesh.h"
#include "Pipeline.h"

#include <Texture.h>

#include <memory>
#include <unordered_map>
#include <vector>

namespace sge {
struct DescriptorSetInfo {
    std::unique_ptr<DescriptorSetLayout> layout;
    std::unique_ptr<Buffer> uboBuffer;
    VkDescriptorSet set;
};

struct PipelineInfo {
    std::string name;
    VkPipelineLayout pipelineLayout;
    std::unique_ptr<Pipeline> pipeline;
};

class MeshMGR {
 public:
    MeshMGR(const MeshMGR&) = delete;
    MeshMGR(MeshMGR&&) = delete;
    MeshMGR& operator=(const MeshMGR&) = delete;
    MeshMGR& operator=(MeshMGR&&) = delete;

    static MeshMGR& Instance() noexcept;
    void setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptorUBO_pool) noexcept;
    void clearTable() noexcept;
    DescriptorPool& getDescriptorPool() const;

    std::vector<PipelineInfo> m_pipelines;
    std::vector<DescriptorSetInfo> m_sets;
    std::vector<Mesh> m_meshes;
    std::vector<Mesh> m_systemMeshes;
    std::unique_ptr<Buffer> m_generalMatrixUBO;
    std::unique_ptr<Buffer> m_debugUBO;
    std::unique_ptr<Buffer> m_normalTestUBO;
    std::unordered_map<std::string, Texture> m_textures;
    std::unique_ptr<DescriptorPool> m_UIPool{};

 private:
    MeshMGR();
    ~MeshMGR();
    std::unique_ptr<DescriptorPool> m_globalPool{};
};
}  // namespace sge
