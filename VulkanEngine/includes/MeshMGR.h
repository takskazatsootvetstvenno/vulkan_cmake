#pragma once
#include "Pipeline.h"
#include "Mesh.h"
#include "Descriptors.h"
#include <vector>
#include <memory>
#include <unordered_map>
#include <Texture.h>

namespace sge {
	struct DescriptorSetInfo
	{
		std::unique_ptr<DescriptorSetLayout> layout;
		std::unique_ptr<Buffer> uboBuffer;
		VkDescriptorSet set;
	};

	struct PipelineInfo
	{
		std::string name;
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
		void setDescriptorPool(std::unique_ptr<DescriptorPool>&& uptr_descriptorUBO_pool) noexcept;
		void clearTable() noexcept;
		DescriptorPool& getDescriptorPool() const;

		std::vector<PipelineInfo>      m_pipelines;
		std::vector<DescriptorSetInfo> m_sets;
		std::vector<Mesh>              m_meshes;
		std::vector<Mesh>              m_systemMeshes;
		std::unique_ptr<Buffer>        m_generalMatrixUBO;
		std::unordered_map<std::string, Texture> m_textures;
	private:
		MeshMGR();
		~MeshMGR();
		std::unique_ptr<DescriptorPool> m_globalPool{};
	};
}