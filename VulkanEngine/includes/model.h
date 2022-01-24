#pragma once
#include "Device.h"
#include <vector>

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace sge {
    class Model {
    public:
        struct Vertex {
            glm::vec2 m_position;
            static std::vector<VkVertexInputBindingDescription> getBindingDescription();
            static std::vector<VkVertexInputAttributeDescription> getAttributeDescription();
        };

        Model(Device& device, const std::vector<Vertex>& vertices);
        ~Model();
        Model(const Model&) = delete;
        Model& operator=(const Model&) = delete;

        void bind(VkCommandBuffer commandBuffer);
        void draw(VkCommandBuffer commandBuffer);
    private:
        void createVertexBuffers(const std::vector<Vertex>& vertices);
        Device& m_device;
        VkBuffer m_vertexBuffer;
        VkDeviceMemory m_vertexBefferMemory;
        uint32_t m_vertexCount;
    };

}  // namespace sge
