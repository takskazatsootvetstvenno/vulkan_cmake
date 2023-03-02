#include "WorkflowConfigLoader.h"

#include "json.hpp"

#include "Logger.h"

#include <VulkanHelpUtils.h>

#include <algorithm>
#include <fstream>
#include <utility>
//
using namespace nlohmann;
using nameValueGroup = std::vector<std::pair<std::string, json>>;

ConfigLoader& ConfigLoader::Instance() noexcept {
    static ConfigLoader configLoader;
    return configLoader;
}

static TextureFormat getTextureFormat(const json& type) {
    if (type == "R8G8B8A8_UNORM") return TextureFormat::R8G8B8A8_UNORM;
    if (type == "R8G8B8A8_SRGB") return TextureFormat::R8G8B8A8_SRGB;
    if (type == "R8G8_UNORM") return TextureFormat::R8G8_UNORM;

    throw(std::runtime_error("Wrong texture format type!"));
}

static ShaderDataType getDataType(const json& type) {
    if (type == "INT") return ShaderDataType::INT;
    if (type == "FLOAT") return ShaderDataType::FLOAT;
    if (type == "VEC2") return ShaderDataType::VEC2;
    if (type == "VEC3") return ShaderDataType::VEC3;
    if (type == "VEC4") return ShaderDataType::VEC4;
    if (type == "MAT3") return ShaderDataType::MAT3;
    if (type == "MAT4") return ShaderDataType::MAT4;
    throw(std::runtime_error("Wrong data type!"));
}

static sge::CompareOp getCompareOperation(const json& type) {
    if (type == "EQUAL") return sge::CompareOp::EQUAL;
    if (type == "GREATER") return sge::CompareOp::GREATER;
    if (type == "GREATER_OR_EQUAL") return sge::CompareOp::GREATER_OR_EQUAL;
    if (type == "LESS") return sge::CompareOp::LESS;
    if (type == "LESS_OR_EQUAL") return sge::CompareOp::LESS_OR_EQUAL;
    if (type == "NEVER") return sge::CompareOp::NEVER;
    if (type == "NOT_EQUAL") return sge::CompareOp::NOT_EQUAL;
    if (type == "ALWAYS") return sge::CompareOp::ALWAYS;
    throw(std::runtime_error("Wrong pipeline compare operation type!"));
}

static sge::CullingMode getCullingMode(const json& type) {
    if (type == "BACK") return sge::CullingMode::BACK;
    if (type == "FRONT") return sge::CullingMode::FRONT;
    if (type == "FRONT_AND_BACK") return sge::CullingMode::FRONT_AND_BACK;
    if (type == "NONE") return sge::CullingMode::NONE;

    throw(std::runtime_error("Wrong pipeline culling mode type!"));
}

static sge::FrontFace getFrontFace(const json& type) {
    if (type == "CLOCKWISE") return sge::FrontFace::CLOCKWISE;
    if (type == "COUNTER_CLOCKWISE") return sge::FrontFace::COUNTER_CLOCKWISE;

    throw(std::runtime_error("Wrong pipeline FrontFace type!"));
}

static PipelineDependency getDependency(const json& type) {
    if (type == "COLOR_ATTACHMENT_WRITE") return PipelineDependency::COLOR_ATTACHMENT_WRITE;
    if (type == "DEPTH_READ") return PipelineDependency::DEPTH_READ;
    if (type == "DEPTH_WRITE") return PipelineDependency::DEPTH_WRITE;
    if (type == "FRAGMENT_SHADER_WRITE") return PipelineDependency::FRAGMENT_SHADER_WRITE;
    if (type == "VERTEX_SHADER_WRITE") return PipelineDependency::VERTEX_SHADER_WRITE;

    throw(std::runtime_error("Wrong pipeline dependency type!"));
}

static void parseConstantBuffer(const nameValueGroup& jsonCB, std::vector<ParsedConstantBuffer>& parsedData) {
    try {
        for (const auto& constantBuffer : jsonCB) {
            ParsedConstantBuffer currentConstantBuffer;
            currentConstantBuffer.m_name = constantBuffer.first;
            const auto& fields = constantBuffer.second["FIELDS"];
            for (const auto& field : fields) {
                ParsedConstantBuffer::Field parsedField = {.name = field["NAME"], .type = getDataType(field["TYPE"])};
                currentConstantBuffer.m_fields.emplace_back(parsedField);
            }
            parsedData.emplace_back(currentConstantBuffer);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing constant buffers! Error: " + std::string(e.what())));
    }
}
static void parseVertexShader(const nameValueGroup& jsonVertexShader, std::vector<ParsedVertexShader>& parsedData) {
    try {
        for (const auto& vertexShader : jsonVertexShader) {
            ParsedVertexShader currentVertexShader;
            currentVertexShader.m_name = vertexShader.first;
            currentVertexShader.m_path = vertexShader.second["PATH"];
            const auto& inputs = vertexShader.second["INPUTS"];
            for (const auto& input : inputs) {
                ParsedVertexShader::Field parsedField = {.name = input["NAME"], .type = getDataType(input["TYPE"])};
                currentVertexShader.m_inputs.emplace_back(parsedField);
            }
            const auto& outputs = vertexShader.second["OUTPUTS"];
            for (const auto& output : outputs) {
                ParsedVertexShader::Field parsedField = {.name = output["NAME"], .type = getDataType(output["TYPE"])};
                currentVertexShader.m_outputs.emplace_back(parsedField);
            }
            const auto& uniforms = vertexShader.second["UNIFORMS"];
            for (const auto& uniform : uniforms) currentVertexShader.m_uniforms.push_back(uniform);
            parsedData.emplace_back(currentVertexShader);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing vertex shaders! Error: " + std::string(e.what())));
    }
}

static void parseFragmentShader(const nameValueGroup& jsonfragmentShader,
                                std::vector<ParsedFragmentShader>& parsedData) {
    try {
        for (const auto& fragmentShader : jsonfragmentShader) {
            ParsedFragmentShader currentfragmentShader;
            currentfragmentShader.m_name = fragmentShader.first;
            currentfragmentShader.m_path = fragmentShader.second["PATH"];
            const auto& inputs = fragmentShader.second["INPUTS"];
            for (const auto& input : inputs) {
                ParsedFragmentShader::Field parsedField = {.name = input["NAME"], .type = getDataType(input["TYPE"])};
                currentfragmentShader.m_inputs.emplace_back(parsedField);
            }
            const auto& outputs = fragmentShader.second["OUTPUTS"];
            for (const auto& output : outputs) {
                ParsedFragmentShader::Field parsedField = {.name = output["NAME"], .type = getDataType(output["TYPE"])};
                currentfragmentShader.m_outputs.emplace_back(parsedField);
            }
            const auto& uniforms = fragmentShader.second["UNIFORMS"];
            for (const auto& uniform : uniforms) currentfragmentShader.m_uniforms.push_back(uniform);
            parsedData.emplace_back(currentfragmentShader);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing fragment shader! Error: " + std::string(e.what())));
    }
}

static void parseAttachmentList(const nameValueGroup& jsonAttachmentList,
                                std::vector<ParsedAttachmentList>& parsedData) {
    try {
        for (const auto& attachmentList : jsonAttachmentList) {
            ParsedAttachmentList currentAttachmentList;
            currentAttachmentList.m_name = attachmentList.first;
            const auto& attachments = attachmentList.second["ATTACHMENTS"];
            for (const auto& attachment : attachments) {
                ParsedAttachmentList::attachment parsedAttachment = {.name = attachment["NAME"],
                                                                     .type = attachment["TYPE"]};
                currentAttachmentList.m_attachments.emplace_back(parsedAttachment);
            }
            parsedData.emplace_back(currentAttachmentList);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing attachmentLists! Error: " + std::string(e.what())));
    }
}

static void parsePipeline(const nameValueGroup& jsonPipeline, std::vector<ParsedPipeline>& parsedData) {
    try {
        for (const auto& pipeline : jsonPipeline) {
            ParsedPipeline currentPipeline;
            currentPipeline.m_name = pipeline.first;
            const auto& shaders = pipeline.second["SHADERS"];
            for (const auto& shader : shaders) currentPipeline.m_shaders.emplace_back(shader);
            const auto& dependencies = pipeline.second["DEPENDENCIES"];
            for (const auto& dependency : dependencies)
                currentPipeline.m_dependencies.emplace_back(getDependency(dependency));
            currentPipeline.m_attachment = pipeline.second["ATTACHMENTS"];
            {
                const auto& culling = pipeline.second["CULLING"];
                currentPipeline.m_culling.cullingMode = getCullingMode(culling["MODE"]);
                currentPipeline.m_culling.frontFace = getFrontFace(culling["FACE"]);
            }
            {
                const auto& depth = pipeline.second["DEPTH"];
                currentPipeline.m_depth.depthTestEnable = depth["TEST_ENABLE"];
                currentPipeline.m_depth.depthWrite = depth["DEPTH_WRITE"];
                currentPipeline.m_depth.m_compareOperation = getCompareOperation(depth["COMPARE_OPERATION"]);
            }

            parsedData.emplace_back(currentPipeline);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing pipelines! Error: " + std::string(e.what())));
    }
}

static void parseWorkflow(const nameValueGroup& jsonWorkflow, std::vector<ParsedWorkflow>& parsedData) {
    try {
        for (const auto& workflow : jsonWorkflow) {
            ParsedWorkflow currentWorkflow;
            currentWorkflow.m_name = workflow.first;
            const auto& pipelines = workflow.second["PIPELINES"];

            for (const auto& pipeline : pipelines) currentWorkflow.m_pipelines.emplace_back(pipeline);
            parsedData.emplace_back(currentWorkflow);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing workflows! Error: " + std::string(e.what())));
    }
}

static void parseTexture(const nameValueGroup& jsonTexture, std::vector<ParsedTexture2D>& parsedData) {
    try {
        for (const auto& texture : jsonTexture) {
            ParsedTexture2D currentTexture;
            currentTexture.m_name = texture.first;
            currentTexture.m_format = getTextureFormat(texture.second["FORMAT"]);
            parsedData.emplace_back(currentTexture);
        }
    } catch (const std::exception& e) {
        throw(std::runtime_error("Error while parsing textures! Error: " + std::string(e.what())));
    }
}

void ConfigLoader::configValidation() {
    for (auto& pipeline : m_pipelines) {
        for (const auto& shader : pipeline.m_shaders) {
            auto itVertex = std::find_if(m_vertexShaders.cbegin(), m_vertexShaders.cend(),
                                         [&shader](auto& vertexShader) { return vertexShader.m_name == shader; });
            auto itFragment = std::find_if(m_fragmentShaders.cbegin(), m_fragmentShaders.cend(),
                                           [&shader](auto& fragmentShader) { return fragmentShader.m_name == shader; });
            if (itVertex == m_vertexShaders.cend() && itFragment == m_fragmentShaders.cend()) {
                LOG_ERROR("Config validation: Wrong shader reference in pipeline! Pipeline: \""
                          << pipeline.m_name << "\"\nCan't find shader with name: \"" << shader << "\"");
                throw(std::runtime_error("Config validation: Wrong shader reference in pipeline!"));
            }
        }
        std::sort(pipeline.m_shaders.begin(), pipeline.m_shaders.end());
        const auto itDuplicate = std::adjacent_find(pipeline.m_shaders.cbegin(), pipeline.m_shaders.cend());
        if (itDuplicate != pipeline.m_shaders.cend()) {
            LOG_ERROR("Config validation: Wrong -- duplication shaders in pipeline! Pipeline: \"" << pipeline.m_name
                                                                                                  << "\"");
            throw(std::runtime_error("Config validation: Wrong -- duplication shaders in pipeline!"));
        }

        const auto it = std::find_if(m_attachmentLists.cbegin(), m_attachmentLists.cend(),
                                     [&pipeline_attachment = pipeline.m_attachment](auto& attachment) {
                                         return attachment.m_name == pipeline_attachment;
                                     });
        if (it == m_attachmentLists.cend()) {
            LOG_ERROR("Config validation: Wrong attachment reference in pipeline! Pipeline: \""
                      << pipeline.m_name << "\"\nAttachment: \"" << pipeline.m_attachment << "\"");
            throw(std::runtime_error("Config validation: Wrong shader reference in pipeline!"));
        }
    }

    for (const auto& vertexShader : m_vertexShaders) {
        std::ifstream openCheck(vertexShader.m_path);
        if (!openCheck.is_open()) {
            LOG_ERROR("Config validation: Wrong -- can't open vertex shader! Vertex Shader: \""
                      << vertexShader.m_name << "\"Path: \"" << vertexShader.m_path << "\"");
            throw(std::runtime_error("Config validation: Wrong -- can't open vertex shader!"));
        }
        openCheck.close();
        auto tempUniformCopy = vertexShader.m_uniforms;
        std::sort(tempUniformCopy.begin(), tempUniformCopy.end());
        const auto itDuplicate = std::adjacent_find(tempUniformCopy.cbegin(), tempUniformCopy.cend());
        if (itDuplicate != tempUniformCopy.cend()) {
            LOG_ERROR("Config validation: Wrong -- duplication uniforms in vertex shader! Vertex Shader: \""
                      << vertexShader.m_name << "\"");
            throw(std::runtime_error("Config validation: Wrong -- duplication shaders in vertex shader!"));
        }
        for (auto& uniform : vertexShader.m_uniforms) {
            auto itBuffer = std::find_if(m_constantBuffers.cbegin(), m_constantBuffers.cend(),
                                         [&uniform](auto& uniformName) { return uniformName.m_name == uniform; });
            if (itBuffer == m_constantBuffers.cend()) {
                LOG_ERROR("Config validation: Wrong uniforms reference in vertex shader! Vertex shader: \""
                          << vertexShader.m_name << "\"\nUniform: \"" << uniform << "\"");
                throw(std::runtime_error(
                    "Config validation: Wrong uniforms reference in vertex shader in vertex shader!"));
            }
        }
    }

    for (const auto& fragmentShader : m_fragmentShaders) {
        std::ifstream openCheck(fragmentShader.m_path);
        if (!openCheck.is_open()) {
            LOG_ERROR("Config validation: Wrong -- can't open fragment shader! Fragment Shader: \""
                      << fragmentShader.m_name << "\"Path: \"" << fragmentShader.m_path << "\"");
            throw(std::runtime_error("Config validation: Wrong -- can't open fragment shader!"));
        }
        auto tempUniformCopy = fragmentShader.m_uniforms;
        std::sort(tempUniformCopy.begin(), tempUniformCopy.end());
        for (auto& uniform : fragmentShader.m_uniforms) {
            const auto itDuplicate = std::adjacent_find(tempUniformCopy.cbegin(), tempUniformCopy.cend());
            if (itDuplicate != tempUniformCopy.cend()) {
                LOG_ERROR("Config validation: Wrong -- duplication uniforms in fragment shader! fragment Shader: \""
                          << fragmentShader.m_name << "\"");
                throw(std::runtime_error("Config validation: Wrong -- duplication shaders in fragment shader!"));
            }
            auto itBuffer = std::find_if(m_constantBuffers.cbegin(), m_constantBuffers.cend(),
                                         [&uniform](auto& uniformName) { return uniformName.m_name == uniform; });
            auto itTexture = std::find_if(m_textures2D.cbegin(), m_textures2D.cend(),
                                          [&uniform](auto& uniformName) { return uniformName.m_name == uniform; });
            if (itBuffer == m_constantBuffers.cend() && itTexture == m_textures2D.cend()) {
                LOG_ERROR("Config validation: Wrong uniform reference in fragment shader! fragment shader: \""
                          << fragmentShader.m_name << "\"\nUniform: \"" << uniform << "\"");
                throw(std::runtime_error("Config validation: Wrong uniform reference in fragment shader!"));
            }
        }
    }
    for (const auto& workflow : m_workflows) {
        for (const auto& pipeline : workflow.m_pipelines) {
            auto itPipeline = std::find_if(m_pipelines.cbegin(), m_pipelines.cend(),
                                           [&pipeline](auto& pipelineName) { return pipelineName.m_name == pipeline; });
            if (itPipeline == m_pipelines.cend()) {
                LOG_ERROR("Config validation: Wrong pipeline reference in workflow! Workflow: \""
                          << workflow.m_name << "\"\n Pipeline: \"" << pipeline << "\"");
                throw(std::runtime_error("Config validation: Wrong pipeline reference in workflow!"));
            }
        }
    }
}

void ConfigLoader::loadAndParseConfigs(std::string_view path) {
    try {
        std::ifstream pipelineInfoFile(path);
        if (!pipelineInfoFile.is_open()) LOG_ERROR("Failed to open config file: " << path.data());
        nlohmann::json data = nlohmann::json::parse(pipelineInfoFile);

        nameValueGroup constantBuffer;
        nameValueGroup vertexShaders;
        nameValueGroup fragmentShaders;
        nameValueGroup attachmentList;
        nameValueGroup pipelines;
        nameValueGroup workflows;
        nameValueGroup textures;

        for (auto& [name, obj] : data.items()) {
            auto& objType = obj["TYPE"];
            if (obj["TYPE"] == "CONSTANT_BUFFER") {
                constantBuffer.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "VERTEX_SHADER") {
                vertexShaders.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "FRAGMENT_SHADER") {
                fragmentShaders.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "COLOR_ATTACHMENT_LIST") {
                attachmentList.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "PIPELINE") {
                pipelines.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "WORKFLOW") {
                workflows.push_back({name, obj});
                continue;
            }
            if (obj["TYPE"] == "TEXTURE_2D") {
                textures.push_back({name, obj});
                continue;
            }
            throw(std::runtime_error("Wrong object name!"));
        }

        parseConstantBuffer(constantBuffer, m_constantBuffers);
        parseVertexShader(vertexShaders, m_vertexShaders);
        parseFragmentShader(fragmentShaders, m_fragmentShaders);
        parseAttachmentList(attachmentList, m_attachmentLists);
        parsePipeline(pipelines, m_pipelines);
        parseWorkflow(workflows, m_workflows);
        parseTexture(textures, m_textures2D);
        configValidation();

    } catch (const std::exception& e) {
        std::string error = "JSON Config parser: In file: \"" + std::string(path) + "\", " + e.what();
        LOG_ERROR(error)
        assert(!"Error in json parser!");
    }
}
