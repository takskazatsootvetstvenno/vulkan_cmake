#pragma once
#include <string_view>
#include <memory>
#include <Buffer.h>
namespace sge
{
	class Texture
	{
	public:
		Texture(const std::string_view texturePath);
		~Texture();

		const void* getData() const noexcept;
		size_t getImageSize() const noexcept;
		int getWidth() const noexcept;
		int getHeight() const noexcept;
		void clearDataOnCPU() noexcept;
		void setTextureImage(VkImage image) noexcept;
		void setTextureImageMemory(VkDeviceMemory imageMemory) noexcept;
		void setImageView(VkImageView imageView) noexcept;
		void setSampler(VkSampler sampler) noexcept;
		VkDescriptorImageInfo getDescriptorInfo() const noexcept;
		VkSampler getSampler() noexcept;
		VkImage getTextureImage() noexcept;
		VkDeviceMemory getTextureImageMemory() noexcept;
		VkImageView getImageView() noexcept;
		std::unique_ptr<Buffer> m_texBuffer;
		
	private:
		void* m_data;
		int m_texWidth;
		int m_texHeight;
		size_t m_imageSize;
		std::string m_texturePath;
		bool m_isCPUdataPresent = true;
		VkImage m_textureImage;
		VkDeviceMemory m_textureImageMemory;
		VkImageView m_imageView;
		VkSampler m_sampler;
	};
}