#include "MeshMGR.h"
namespace sge {
void MeshMGR::setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptorUBO_pool) noexcept {
    m_globalPool = std::move(uptr_descriptorUBO_pool);
}

void MeshMGR::clearTable() noexcept {
    m_meshes.clear();
    m_systemMeshes.clear();
    m_pipelines.clear();
    m_sets.clear();
    m_generalMatrixUBO = nullptr;
    m_debugUBO = nullptr;
    m_normalTestUBO = nullptr;
    m_globalPool = {nullptr};
    m_UIPool = {nullptr};
}

DescriptorPool& MeshMGR::getDescriptorPool() const { return *m_globalPool; }

/*static*/ MeshMGR& MeshMGR::Instance() noexcept {
    static MeshMGR meshManager;
    return meshManager;
}

MeshMGR::~MeshMGR() { clearTable(); }

MeshMGR::MeshMGR() {}
}  // namespace sge
