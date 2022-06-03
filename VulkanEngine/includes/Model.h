#pragma once
#include "Device.h"
#include "Mesh.h"
#include <vector>
#include <unordered_map>
#include "Texture.h"
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include <glm/glm.hpp>

namespace sge {
    class Model {
    public:
        Model(Device& device);
        ~Model();
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;
        Model(Model&&) = default;
        Model& operator=(Model&&) = default;
        void bind(const VkCommandBuffer commandBuffer, const Mesh& mesh) const noexcept;
        void draw(const VkCommandBuffer commandBuffer, const Mesh& mesh) const noexcept;
        void createBuffers() noexcept;
        void createTexture(Texture& texture) noexcept;
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices, Mesh& mesh) noexcept;
        void createIndexBuffers(const std::vector<uint32_t>& indices, Mesh& mesh) noexcept;
        Device& m_device;
    };
}
