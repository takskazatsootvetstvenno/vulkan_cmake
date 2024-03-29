#pragma once
#include <vulkan/vulkan_core.h>

#include <array>
#include <memory>
#include <string_view>
#include <vector>
namespace sge {
class Texture {
 public:
    struct CubemapData {
        const std::string frontTexturePath;
        const std::string backTexturePath;
        const std::string topTexturePath;
        const std::string bottomTexturePath;
        const std::string leftTexturePath;
        const std::string rightTexturePath;
    };
    enum class TextureType { Texture2D = 0, Cubemap };
    Texture(const std::string_view texturePath);
    Texture(CubemapData&& texturePath);
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
    bool isProcessed() const noexcept;
    std::array<void*, 6> getCubemapData() const noexcept;
    const TextureType getTextureType() const noexcept;
    VkDescriptorImageInfo getDescriptorInfo() const noexcept;
    VkSampler getSampler() noexcept;
    VkImage getTextureImage() noexcept;
    VkDeviceMemory getTextureImageMemory() noexcept;
    VkImageView getImageView() noexcept;

 private:
    void* m_data;
    std::vector<uint8_t> m_dataCubemap;
    std::array<void*, 6> m_cubemapData;
    int m_texWidth;
    int m_texHeight;
    size_t m_imageSize;
    std::string m_texturePath;
    CubemapData m_cubemapPath;
    TextureType m_textureType = TextureType::Texture2D;
    bool m_isCPUdataPresent = true;
    VkImage m_textureImage = nullptr;
    VkDeviceMemory m_textureImageMemory = nullptr;
    VkImageView m_imageView = nullptr;
    VkSampler m_sampler = nullptr;
};
}  // namespace sge
