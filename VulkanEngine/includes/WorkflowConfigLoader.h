#include "VulkanHelpUtils.h"

#include <array>
#include <string>
#include <vector>

struct ParsedConstantBuffer {
    struct Field {
        std::string name;
        ShaderDataType type;
    };
    std::vector<Field> m_fields;
    std::string m_name;
};

struct ParsedTexture2D {
    TextureFormat m_format; 
    std::string m_name;
};

struct ParsedVertexShader {
    struct Field {
        std::string name;
        ShaderDataType type;
    };

    std::vector<Field> m_inputs;
    std::vector<Field> m_outputs;
    std::vector<std::string> m_uniforms;
    std::string m_name;
    std::string m_path;
};

struct ParsedFragmentShader {
    struct Field {
        std::string name;
        ShaderDataType type;
    };

    std::vector<Field> m_inputs;
    std::vector<Field> m_outputs;
    std::vector<std::string> m_uniforms;
    std::string m_name;
    std::string m_path;
};

struct ParsedAttachmentList {
    struct attachment {
        std::string name;
        std::string type;
    };
    std::vector<attachment> m_attachments;
    std::string m_name;
};

struct ParsedPipeline {
    struct Culling {
        sge::CullingMode cullingMode;
        sge::FrontFace frontFace;
    };
    struct Depth {
        sge::CompareOp m_compareOperation;
        bool depthTestEnable;
        bool depthWrite;
    };
    std::vector<std::string> m_shaders;
    std::vector<PipelineDependency> m_dependencies;
    Depth m_depth;
    Culling m_culling;
    std::string m_attachment;
    std::string m_name;
};

struct ParsedWorkflow {
    std::vector<std::string> m_pipelines;
    std::string m_name;
};

class ConfigLoader {
 public:
    static ConfigLoader& Instance() noexcept;
    void loadAndParseConfigs(std::string_view path = "C:\\Users\\Denis\\source\\repos\\vulkan_cmake\\Pipelines.json");

 private:
    void configValidation();
    std::vector<ParsedConstantBuffer> m_constantBuffers;
    std::vector<ParsedVertexShader> m_vertexShaders;
    std::vector<ParsedFragmentShader> m_fragmentShaders;
    std::vector<ParsedAttachmentList> m_attachmentLists;
    std::vector<ParsedPipeline> m_pipelines;
    std::vector<ParsedWorkflow> m_workflows;
    std::vector<ParsedTexture2D> m_textures2D;
};
