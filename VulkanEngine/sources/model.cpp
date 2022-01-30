#include "model.h"
namespace sge {
	Model::~Model()
	{
		vkDestroyBuffer(m_device.device(), m_vertexBuffer, nullptr);
		vkFreeMemory(m_device.device(), m_vertexBefferMemory, nullptr);
	}
	void Model::bind(VkCommandBuffer commandBuffer) const noexcept
	{
		VkBuffer buffers[] = { m_vertexBuffer };
		VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
	}

	void Model::draw(VkCommandBuffer commandBuffer) const noexcept
	{
		vkCmdDraw(commandBuffer, m_vertexCount, 1, 0, 0);
	}
	void Model::createVertexBuffers(const std::vector<Vertex>& vertices)
	{
		m_vertexCount = static_cast<uint32_t>(vertices.size());
		assert(m_vertexCount >= 3 && "Vertex count must be at least 3");
		VkDeviceSize bufferSize = sizeof(vertices[0]) * m_vertexCount;
		
		m_device.createBuffer(
			bufferSize,
			VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
			VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			m_vertexBuffer,
			m_vertexBefferMemory);
		void* data;
		vkMapMemory(m_device.device(), m_vertexBefferMemory, 0, bufferSize, 0, &data);		
		memcpy(data, vertices.data(), static_cast<size_t>(bufferSize));
		vkUnmapMemory(m_device.device(), m_vertexBefferMemory);
	}

	/*static*/ std::vector<VkVertexInputBindingDescription> Model::Vertex::getBindingDescription() {
		std::vector<VkVertexInputBindingDescription> bindingDescriptions(1);
		bindingDescriptions[0].binding = 0;
		bindingDescriptions[0].stride = sizeof(Vertex);
		bindingDescriptions[0].inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
		return bindingDescriptions;
	}
	/*static*/ std::vector<VkVertexInputAttributeDescription>  Model::Vertex::getAttributeDescription()
	{
		std::vector<VkVertexInputAttributeDescription> attributeDescriptions(1);
		attributeDescriptions[0].binding = 0;
		attributeDescriptions[0].location = 0;
		attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
		attributeDescriptions[0].offset = 0;
		return attributeDescriptions;

	}
	Model::Model(Device& device, const std::vector<Vertex>& vertices)
		:m_device(device)
	{
		createVertexBuffers(vertices);
	}

}