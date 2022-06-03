#include "MeshMGR.h"
namespace sge {
	void MeshMGR::setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptorUBO_pool) noexcept
	{
		m_globalPool = std::move(uptr_descriptorUBO_pool);
	}
	void MeshMGR::setDescriptorSamplerPool(std::unique_ptr<DescriptorPool>&& uptr_descriptorSampler_pool) noexcept
	{
		m_globalSamplerPool = std::move(uptr_descriptorSampler_pool);
	}
	void MeshMGR::clearTable() noexcept
	{
		m_meshes.clear();
		m_pipelines.clear();
		m_sets.clear();
		m_generalMatrixUBO = nullptr;
		setDescriptorPool(nullptr);
		setDescriptorSamplerPool(nullptr);
	}
	DescriptorPool& MeshMGR::getDescriptorPool() const
	{
		return *m_globalPool;
	}
	DescriptorPool& MeshMGR::getDescriptorSamplerPool() const
	{
		return *m_globalSamplerPool;
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