#pragma once
#include "Pipeline.h"
#include "Mesh.h"
#include "Descriptors.h"
#include <vector>
#include <memory>

namespace sge {
	struct DescriptorSetInfo
	{
		std::unique_ptr<DescriptorSetLayout> layout;
		std::unique_ptr<Buffer> uboBuffer;
		VkDescriptorSet set;
	};
	struct PipelineInfo
	{
		VkPipelineLayout pipelineLayout;
		std::unique_ptr<Pipeline> pipeline;
	};

	class MeshMGR
	{
	public:
		MeshMGR(const MeshMGR&) = delete;
		MeshMGR(MeshMGR&&) = delete;
		MeshMGR& operator=(const MeshMGR&) = delete;
		MeshMGR& operator=(MeshMGR&&) = delete;

		static MeshMGR& Instance() noexcept;
		void setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptor_pool) noexcept;
		void clearTable() noexcept;
		DescriptorPool& getDescriptorPool() const;
		std::vector<PipelineInfo>      m_pipelines;
		std::vector<DescriptorSetInfo> m_sets;
		std::vector<Mesh>              m_meshes;
		std::unique_ptr<Buffer>        m_generalMatrixUBO;
	private:
		MeshMGR();
		~MeshMGR();
		std::unique_ptr<DescriptorPool> m_globalPool{};
	};
}