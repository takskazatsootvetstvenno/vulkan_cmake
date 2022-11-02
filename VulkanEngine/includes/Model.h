#pragma once
#include "Device.h"
#include "Mesh.h"
#include "Texture.h"

#include <unordered_map>
#include <vector>

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
}  // namespace sge
