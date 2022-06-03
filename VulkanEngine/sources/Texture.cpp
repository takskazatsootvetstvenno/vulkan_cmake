#define STB_IMAGE_IMPLEMENTATION
#include <Texture.h>
#include <stb_image.h>
#include "Logger.h"

namespace sge
{
	Texture::Texture(const std::string_view texturePath)
		:m_texturePath(texturePath)
	{
		int texChannels;
		stbi_set_flip_vertically_on_load(true);
		m_data = stbi_load(m_texturePath.c_str(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_data)
		{
			assert(false);
			LOG_ERROR("Failed to load texture: " << m_texturePath << "!")
		}
		m_imageSize = static_cast<size_t>(m_texWidth) * m_texHeight * 4;
	}

	const void* Texture::getData() const noexcept
	{
		return m_data;
	}

	size_t Texture::getImageSize() const noexcept
	{
		return m_imageSize;
	}

	int Texture::getWidth() const noexcept
	{
		return m_texWidth;
	}

	int Texture::getHeight() const noexcept
	{
		return m_texHeight;
	}

	void Texture::clearDataOnCPU() noexcept
	{
		stbi_image_free(m_data);
		m_isCPUdataPresent = false;
	}

	void Texture::setTextureImage(VkImage image) noexcept
	{
		m_textureImage = image;
	}

	void Texture::setTextureImageMemory(VkDeviceMemory imageMemory) noexcept
	{
		m_textureImageMemory = imageMemory;
	}

	VkImage Texture::getTextureImage() noexcept
	{
		return m_textureImage;
	}

	VkDeviceMemory Texture::getTextureImageMemory() noexcept
	{
		return m_textureImageMemory;
	}

	VkImageView Texture::getImageView() noexcept
	{
		return m_imageView;
	}

	void Texture::setImageView(VkImageView imageView) noexcept
	{
		m_imageView = imageView;
	}

	void Texture::setSampler(VkSampler sampler) noexcept
	{
		m_sampler = sampler;
	}

	VkDescriptorImageInfo Texture::getDescriptorInfo() const noexcept
	{
		return
		{
			.sampler = m_sampler,
			.imageView = m_imageView,
			.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
		};
	}

	VkSampler Texture::getSampler() noexcept
	{
		return m_sampler;
	}

	Texture::~Texture()
	{                                     
		if (m_isCPUdataPresent) stbi_image_free(m_data);
	}
}