#include "model.h"
#include <string>
#include <chrono>
#include "App.h"
#include "Buffer.h"
#include "MeshMGR.h"
#include <memory>
using namespace std::chrono;
namespace sge {
	static PFN_vkSetDebugUtilsObjectNameEXT SetDebugUtilsObjectNameEXT;
	Model::~Model()
	{
	}

	void Model::bind(const VkCommandBuffer commandBuffer, const Mesh& mesh) const noexcept
	{
		const VkBuffer buffers[] = { mesh.m_vertexBuffer->getBuffer() };
		const VkDeviceSize offsets[] = { 0 };
		vkCmdBindVertexBuffers(commandBuffer, 0, 1, buffers, offsets);
		vkCmdBindIndexBuffer(commandBuffer, mesh.m_indexBuffer->getBuffer(), 0, VK_INDEX_TYPE_UINT32);
	}

	void Model::draw(const VkCommandBuffer commandBuffer, const Mesh& mesh) const noexcept
	{
		assert(mesh.getIndexCount() != 0 && "Mesh must use index drawing");
		vkCmdDrawIndexed(commandBuffer, mesh.getIndexCount(), 1, 0, 0, 0);
	}

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices, Mesh& mesh)
	{
		const uint32_t vertexSize = sizeof(vertices[0]);
		const VkDeviceSize bufferSize = static_cast<uint64_t>(vertexSize) * mesh.getVertexCount();
		
		Buffer stagingBuffer{
			 m_device,
			 vertexSize,
			 mesh.getVertexCount(),
			 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};
		stagingBuffer.map();
		stagingBuffer.writeToBuffer(vertices.data());
		mesh.m_vertexBuffer = std::make_unique<Buffer>(
			 m_device,
			 vertexSize,
			 mesh.getVertexCount(),
			 VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		m_device.copyBuffer(stagingBuffer.getBuffer(), mesh.m_vertexBuffer->getBuffer(), bufferSize);
	}

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices, Mesh& mesh)
	{
		assert(indices.empty() == 0 && "Mesh must use index drawing");

		const uint32_t indexSize = sizeof(indices[0]);
		const VkDeviceSize bufferSize = static_cast<uint64_t>(indexSize) * mesh.getIndexCount();

		Buffer stagingBuffer{
			 m_device,
			 indexSize,
			 mesh.getIndexCount(),
			 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
			 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
		};

		stagingBuffer.map();
		stagingBuffer.writeToBuffer(indices.data());
		mesh.m_indexBuffer = std::make_unique<Buffer>(
			 m_device,
			 indexSize,
			 mesh.getIndexCount(),
			 VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT,
			 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		);
		m_device.copyBuffer(stagingBuffer.getBuffer(), mesh.m_indexBuffer->getBuffer(), bufferSize);

	}

	Model::Model(Device& device)
		:m_device(device)
	{
		auto& meshes = MeshMGR::Instance().m_meshes;
		for (auto& mesh : meshes) {
			createVertexBuffers(mesh.m_pos, mesh);
			createIndexBuffers(mesh.m_ind, mesh);
		}
	}

}