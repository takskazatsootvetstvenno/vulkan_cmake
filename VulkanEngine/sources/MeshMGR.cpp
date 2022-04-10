#include "MeshMGR.h"
namespace sge {
	void MeshMGR::setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptor_pool) noexcept
	{
		m_globalPool = std::move(uptr_descriptor_pool);
	}
	void MeshMGR::clearTable() noexcept
	{
		m_meshes.clear();
		m_pipelines.clear();
		m_sets.clear();
		m_generalMatrixUBO = nullptr;
		setDescriptorPool(nullptr);
	}
	DescriptorPool& MeshMGR::getDescriptorPool() const
	{
		return *m_globalPool;
	}
	/*static*/ MeshMGR& MeshMGR::Instance() noexcept
	{
		static MeshMGR meshManager;
		return meshManager;
	}

	MeshMGR::~MeshMGR()
	{
		clearTable();
	}
	MeshMGR::MeshMGR()
	{

	}
}