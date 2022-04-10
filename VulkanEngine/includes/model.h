#pragma once
#include "Device.h"
#include "Mesh.h"
#include <vector>

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
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices, Mesh& mesh);
        void createIndexBuffers(const std::vector<uint32_t>& indices, Mesh& mesh);
        Device& m_device;
    };
}
