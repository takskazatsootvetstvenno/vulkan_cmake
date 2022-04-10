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
		return *this;
	}
	Mesh::Mesh(Mesh&& other) noexcept
		:
		m_ind(std::move(other.m_ind)),
		m_pos(std::move(other.m_pos)),
		m_indexBuffer(std::move(other.m_indexBuffer)),
		m_vertexBuffer(std::move(other.m_vertexBuffer)),
		m_modelMatrix(std::move(other.m_modelMatrix)),
		m_material(std::move(other.m_material))
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
	/*static*/ std::vector<VkVertexInputBindingDescription> Vertex::getBindingDescription() noexcept{
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}
	/*static*/ std::vector<VkVertexInputAttributeDescription>  Vertex::getAttributeDescription() noexcept
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(2);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32B32_SFLOAT;//VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = 0;

		attributeDescriptions[1].binding = 0;
		attributeDescriptions[1].location = 1;
		attributeDescriptions[1].format = VK_FORMAT_R32G32B32_SFLOAT;//VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[1].offset = sizeof(Vertex::m_position);
		return attributeDescriptions;
	}


}