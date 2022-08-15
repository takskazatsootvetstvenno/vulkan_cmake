#include "Mesh.h"
namespace sge {
	Mesh& Mesh::operator=(Mesh&& other) noexcept
	{
		m_ind = std::move(other.m_ind);
		m_pos = std::move(other.m_pos);
		m_indexBuffer = std::move(other.m_indexBuffer);
		m_vertexBuffer = std::move(other.m_vertexBuffer);
		m_modelMatrix = std::move(other.m_modelMatrix);
		m_material = std::move(other.m_material);
		m_materialType = std::move(other.m_materialType);
		m_pipelineId = std::move(other.m_pipelineId);
		m_descriptorSetId = std::move(other.m_descriptorSetId);
		m_name = std::move(other.m_name);
		return *this;
	}
	Mesh::Mesh(Mesh&& other) noexcept
		:
		m_ind(std::move(other.m_ind)),
		m_pos(std::move(other.m_pos)),
		m_indexBuffer(std::move(other.m_indexBuffer)),
		m_vertexBuffer(std::move(other.m_vertexBuffer)),
		m_modelMatrix(std::move(other.m_modelMatrix)),
		m_material(std::move(other.m_material)),
		m_materialType(std::move(other.m_materialType)),
		m_pipelineId(std::move(other.m_pipelineId)),
		m_descriptorSetId(std::move(other.m_descriptorSetId)),
		m_name(std::move(other.m_name))
	{
	}
	void Mesh::setModelMatrix(const glm::mat4& matrix)
	{
		m_modelMatrix = matrix;
	} 
	void Mesh::setModelMatrix(glm::mat4&& matrix)
	{
		m_modelMatrix = std::move(matrix);
	}
	void Mesh::setName(const std::string_view newName) noexcept
	{
		m_name = newName;
	}
	const glm::mat4& Mesh::getModelMatrix() const
	{
		return m_modelMatrix;
	}
	uint32_t Mesh::getVertexCount() const
	{
		return m_pos.size();
	}
	uint32_t Mesh::getIndexCount() const
	{
		return m_ind.size();
	}
	uint32_t Mesh::getPipelineId() const
	{
		return m_pipelineId;
	}
	uint32_t Mesh::getDescriptorSetId() const
	{
		return m_descriptorSetId;
	}
	const std::string& Mesh::getName() const noexcept
	{
		return m_name;
	}
	/*static*/ std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescription() noexcept{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions =
		{
			{
				.binding = 0,
				.stride = sizeof(Vertex),
				.inputRate = VK_VERTEX_INPUT_RATE_VERTEX
			}
		};
		return bindingDescriptions;
	}
	/*static*/ std::vector<VkVertexInputAttributeDescription>  Vertex::getAttributeDescription() noexcept
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions =
		{
			{
				.location = 0,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, m_position)
			},
			{
				.location = 1,
				.binding = 0,
				.format = VK_FORMAT_R32G32B32_SFLOAT,
				.offset = offsetof(Vertex, m_normal)
			},
			{
				.location = 2,
				.binding = 0,
				.format = VK_FORMAT_R32G32_SFLOAT,
				.offset = offsetof(Vertex, m_UV)
			},
		};

		return attributeDescriptions;
	}


}