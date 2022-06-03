#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#define GLM_FORCE_DEFAULT_ALIGNED_GENTYPES
#include "Device.h"
#include "Buffer.h"
#include <vector>
#include <memory>

#include <glm/glm.hpp>

namespace sge {

    struct Vertex {
        glm::vec3 m_position;
        glm::vec3 m_normal;
        glm::vec2 m_UV;
        static std::vector<VkVertexInputBindingDescription> getBindingDescription() noexcept;
        static std::vector<VkVertexInputAttributeDescription> getAttributeDescription() noexcept;
    };

    class Mesh {
    public:
        struct Material
        {
            glm::vec4 m_baseColor{ 1.f };
            glm::vec3 m_emissiveFactor{ 1.f };
            float     m_metallicFactor = 0.f;
            float     m_roughnessFactor = 0.f;

            bool      m_hasColorMap = false;
            bool      m_hasMetallicRoughnessMap = false;
            bool      m_hasNormalMap = false;
            bool      m_hasEmissiveMap= false;

            std::string m_baseColorPath;
            std::string m_MetallicRoughnessPath;
            std::string m_NormalPath;
            std::string m_EmissivePath;
        };

        enum class MaterialType {
            PBR,
            Phong
        };

        Mesh()  = default;
        ~Mesh() = default;
        Mesh& operator=(const Mesh&) = delete;
        Mesh(const Mesh&) = delete;
        Mesh& operator=(Mesh&& other) noexcept;
        Mesh(Mesh&& other) noexcept;

        void setModelMatrix(const glm::mat4& matrix);
        void setModelMatrix(glm::mat4&& matrix);
        const glm::mat4& getModelMatrix() const;
        uint32_t getVertexCount() const;
        uint32_t getIndexCount() const;
        uint32_t getPipelineId() const;
        uint32_t getDescriptorSetId() const;
        std::vector<Vertex>     m_pos;
        std::vector<uint32_t>   m_ind;
        std::unique_ptr<Buffer> m_vertexBuffer;
        std::unique_ptr<Buffer> m_indexBuffer;

        uint32_t  m_pipelineId = 0;
        uint32_t  m_descriptorSetId = 0;
        Material  m_material;
        MaterialType m_materialType{MaterialType::Phong};
    private:
        glm::mat4 m_modelMatrix{ 1.f };
    };
}