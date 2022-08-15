#include "Model.h"
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

	void Model::createBuffers() noexcept
	{
		auto& meshes = MeshMGR::Instance().m_meshes;
		for (auto& mesh : meshes) {
			createVertexBuffers(mesh.m_pos, mesh);
			createIndexBuffers(mesh.m_ind, mesh);
		}
		for (auto& mesh : MeshMGR::Instance().m_systemMeshes) {
			createVertexBuffers(mesh.m_pos, mesh);
			createIndexBuffers(mesh.m_ind, mesh);
		}
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

	void Model::createVertexBuffers(const std::vector<Vertex>& vertices, Mesh& mesh) noexcept
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

	void Model::createIndexBuffers(const std::vector<uint32_t>& indices, Mesh& mesh) noexcept
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

	void Model::createTexture(Texture& texture) noexcept
	{
			Buffer stagingBuffer{
				 m_device,
				 texture.getImageSize(),
				 1,
				 VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
				 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
			};

			stagingBuffer.map();
			stagingBuffer.writeToBuffer(texture.getData());

			texture.clearDataOnCPU();

			VkImage textureImage;
			VkDeviceMemory textureImageMemory;

			uint32_t arrLayers = 1;
			switch (texture.getTextureType())
			{
			case Texture::TextureType::Texture2D:
					arrLayers = 1;
				break;
			case Texture::TextureType::Cubemap:
					arrLayers = 6;
				break;
			default:
				break;
			}
			VkImageCreateFlags image_flags = (texture.getTextureType() == Texture::TextureType::Cubemap) ? VK_IMAGE_CREATE_CUBE_COMPATIBLE_BIT : 0;
			VkImageCreateInfo imageInfo{};
			imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
			imageInfo.imageType = VK_IMAGE_TYPE_2D;
			imageInfo.extent.width = texture.getWidth();
			imageInfo.extent.height = texture.getHeight();
			imageInfo.extent.depth = 1;
			imageInfo.mipLevels = 1;
			imageInfo.arrayLayers = arrLayers;
			imageInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
			imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
			imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
			imageInfo.usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
			imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
			imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
			imageInfo.flags = image_flags;

			m_device.createImageWithInfo(
				imageInfo,
				VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
				textureImage,
				textureImageMemory);

			m_device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, arrLayers);
			m_device.copyBufferToImage(stagingBuffer.getBuffer(), textureImage, static_cast<uint32_t>(texture.getWidth()), static_cast<uint32_t>(texture.getHeight()), arrLayers);
			m_device.transitionImageLayout(textureImage, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, arrLayers);

			auto textureImageView = m_device.createImageView(textureImage,
				VK_FORMAT_R8G8B8A8_SRGB,
				texture.getTextureType() == Texture::TextureType::Cubemap);

			VkSamplerCreateInfo samplerInfo{};
			samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
			samplerInfo.magFilter = VK_FILTER_LINEAR;
			samplerInfo.minFilter = VK_FILTER_LINEAR;
			samplerInfo.addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT;
			samplerInfo.anisotropyEnable = VK_TRUE;
			samplerInfo.maxAnisotropy = m_device.getPhysicalDeviceProperties().limits.maxSamplerAnisotropy;
			samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
			samplerInfo.unnormalizedCoordinates = VK_FALSE;
			samplerInfo.compareEnable = VK_FALSE;
			samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
			samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
			samplerInfo.mipLodBias = 0.0f;
			samplerInfo.minLod = 0.0f;
			samplerInfo.maxLod = 0.0f;

			auto sampler = m_device.createTextureSampler(samplerInfo);

			texture.setTextureImage(textureImage);
			texture.setImageView(textureImageView);
			texture.setTextureImageMemory(textureImageMemory);
			texture.setSampler(sampler);
		
	}

	Model::Model(Device& device)
		:m_device(device)
	{
	}

}
