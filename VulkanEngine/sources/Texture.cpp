#define STB_IMAGE_IMPLEMENTATION
#include <Texture.h>
#include <stb_image.h>
#include "Logger.h"

namespace sge
{
	enum CubemapTextures
	{
		back = 0,
		front = 1,
		bottom = 2,
		top = 3,
		right = 4,
		left = 5
	};

	Texture::Texture(const std::string_view texturePath)
		:m_texturePath(texturePath),
		m_cubemapData{}
	{
		int texChannels;
		stbi_set_flip_vertically_on_load(false);
		m_data = stbi_load(m_texturePath.c_str(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_data)
		{
			assert(false);
			LOG_ERROR("Failed to load texture: " << m_texturePath << "!")
		}
		m_imageSize = static_cast<size_t>(m_texWidth) * m_texHeight * 4;
		m_textureType = TextureType::Texture2D;
	}

	Texture::Texture(CubemapData&& texturePath)
		:m_cubemapPath(std::move(texturePath))
	{
		int texChannels;
		stbi_set_flip_vertically_on_load(true);
		m_cubemapData[CubemapTextures::back] = stbi_load(m_cubemapPath.backTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[CubemapTextures::back]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.backTexturePath << "!")
			assert(false);
		}
		m_cubemapData[front] = stbi_load(m_cubemapPath.frontTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[front]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.frontTexturePath << "!")
			assert(false);
		}
		m_cubemapData[left] = stbi_load(m_cubemapPath.leftTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[left]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.leftTexturePath << "!")
			assert(false);
		}
		m_cubemapData[right] = stbi_load(m_cubemapPath.rightTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[right]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.rightTexturePath << "!")
			assert(false);
		}
		m_cubemapData[bottom] = stbi_load(m_cubemapPath.bottomTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[bottom]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.bottomTexturePath << "!")
			assert(false);
		}
		m_cubemapData[top] = stbi_load(m_cubemapPath.topTexturePath.data(), &m_texWidth, &m_texHeight, &texChannels, STBI_rgb_alpha);
		if (!m_cubemapData[top]) {
			LOG_ERROR("Failed to load texture: " << m_cubemapPath.topTexturePath << "!")
			assert(false);
		}
		m_imageSize = static_cast<size_t>(m_texWidth) * m_texHeight * 4 * 6;
		size_t singleImageSize = static_cast<size_t>(m_texWidth) * m_texHeight * 4;
		m_dataCubemap.resize(m_imageSize);
		for (int i = 0; i < 6; ++i)
			memcpy(m_dataCubemap.data() + singleImageSize * i, m_cubemapData[i], singleImageSize);
		m_data = m_dataCubemap.data();
		for (int i = 0; i < 6; ++i)
			stbi_image_free(m_cubemapData[i]);
		m_textureType = TextureType::Cubemap;
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
		switch (m_textureType)
		{
		case sge::Texture::TextureType::Texture2D:
			stbi_image_free(m_data);
			break;
		case sge::Texture::TextureType::Cubemap:
			m_dataCubemap.clear();
			m_dataCubemap.shrink_to_fit();
			break;
		}
		
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

	bool Texture::isProcessed() const noexcept
	{
		return (m_sampler != nullptr) && (m_imageView != nullptr) && (m_textureImage != nullptr);
	}

	std::array<void*, 6> Texture::getCubemapData() const noexcept
	{
		assert(m_textureType == TextureType::Cubemap);
		return m_cubemapData;
	}

	const Texture::TextureType Texture::getTextureType() const noexcept
	{
		return m_textureType;
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