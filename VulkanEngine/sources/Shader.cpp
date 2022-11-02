#include "Shader.h"

#include "Logger.h"

#include <shaderc/shaderc.hpp>

#include <cassert>
#include <fstream>

namespace sge {
std::string Shader::readFile(const std::string_view filePath) noexcept {
    std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
    if (!file.is_open()) {
        LOG_ERROR("Failed to open file: " << filePath << "!")
        m_isValid = false;
        assert(false);
        return "";
    }
    const size_t fileSize = static_cast<size_t>(file.tellg());
    std::string buffer;
    buffer.resize(fileSize);
    file.seekg(0);
    file.read(buffer.data(), fileSize);
    file.close();
    return buffer;
}

bool Shader::processShader(const ShaderType type, bool isHLSL) noexcept {
    shaderc::CompileOptions compilerOptions;
    compilerOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
    compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
    compilerOptions.SetTargetSpirv(shaderc_spirv_version_1_5);
    compilerOptions.SetWarningsAsErrors();
    compilerOptions.SetGenerateDebugInfo();

    const shaderc_source_language lang = isHLSL ? shaderc_source_language_hlsl : shaderc_source_language_glsl;
    compilerOptions.SetSourceLanguage(lang);
    shaderc::SpvCompilationResult result;
    switch (shaderc::Compiler compiler; type) {
        case ShaderType::VertexShader:
            result = compiler.CompileGlslToSpv(m_vertShader, shaderc_vertex_shader, "sh.vert", "main", compilerOptions);
            m_vertexShaderSpirV = {result.cbegin(), result.cend()};
            break;
        case ShaderType::FragmentShader:
            result = compiler.CompileGlslToSpv(m_fragShader, shaderc_fragment_shader, "sh.frag", "main",
                                               compilerOptions);
#if 0
			{auto preresult = compiler.PreprocessGlsl(m_fragShader, shaderc_fragment_shader, "sh.frag", compilerOptions);
			LOG_MSG(std::string{ preresult.begin(), preresult.end() });
		}
#endif
            m_fragShaderSpirV = {result.cbegin(), result.cend()};
            break;
        case ShaderType::GeometryShader:
            result = compiler.CompileGlslToSpv(m_geometryShader, shaderc_geometry_shader, "sh.geom", "main",
                                               compilerOptions);
            m_geometryShaderSpirV = {result.cbegin(), result.cend()};
    }
    if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
        LOG_ERROR("Error message: " << result.GetErrorMessage());
        LOG_ERROR("Shader compile status: " << result.GetCompilationStatus());
        return false;
    }
    return true;
}

bool Shader::compileShaders() noexcept {
    const auto vertexExtenstionPos = m_vertShaderPath.rfind('.');
    if (vertexExtenstionPos == std::string::npos) {
        LOG_ERROR("Can't open vertex shader file without extenstion!\nFile: " << m_vertShaderPath);
        return (false);
    }

    const auto fragmentExtenstionPos = m_fragShaderPath.rfind('.');
    if (fragmentExtenstionPos == std::string::npos) {
        LOG_ERROR("Can't open fragment shader file without extenstion!\nFile: " << m_fragShaderPath);
        return (false);
    }

    const auto geometryExtenstionPos = m_geometryShaderPath.rfind('.');
    if (!m_geometryShaderPath.empty()) {
        if (geometryExtenstionPos == std::string::npos) {
            LOG_ERROR("Can't open geometry shader file without extenstion!\nFile: " << m_geometryShaderPath);
            return (false);
        }
    }

    bool isHLSL = (m_vertShaderPath.substr(vertexExtenstionPos) == ".hlsl");

    if (auto vertShaderText = readFile(m_vertShaderPath); !isHLSL) {
        auto versionPos = vertShaderText.find("#version ");
        m_vertShader = vertShaderText.substr(0, versionPos + 12) + "\n" + m_defines.vertShaderDefines +
                       vertShaderText.substr(versionPos + 12);
    } else
        m_vertShader = m_defines.vertShaderDefines + vertShaderText;

    if (processShader(ShaderType::VertexShader, isHLSL) == false) return false;

    isHLSL = (m_fragShaderPath.substr(fragmentExtenstionPos) == ".hlsl");

    if (auto fragShaderText = readFile(m_fragShaderPath); !isHLSL) {
        auto versionPos = fragShaderText.find("#version ");
        if (versionPos == std::string::npos) {
            LOG_ERROR("Can't found \"#version\" in fragment shader!\nFile: " << m_fragShaderPath);
            return (false);
        }
        m_fragShader = fragShaderText.substr(0, versionPos + 12) + "\n" + m_defines.fragmentShaderDefines +
                       fragShaderText.substr(versionPos + 12);
    } else
        m_fragShader = m_defines.fragmentShaderDefines + fragShaderText;

    if (processShader(ShaderType::FragmentShader, isHLSL) == false) return false;

    if (!m_geometryShaderPath.empty()) {
        isHLSL = (m_geometryShaderPath.substr(geometryExtenstionPos) == ".hlsl");

        if (auto geometryShaderText = readFile(m_geometryShaderPath); !isHLSL) {
            auto versionPos = geometryShaderText.find("#version ");
            if (versionPos == std::string::npos) {
                LOG_ERROR("Can't found \"#version\" in geometry shader!\nFile: " << m_geometryShaderPath);
                return (false);
            }
            m_geometryShader = geometryShaderText.substr(0, versionPos + 12) + "\n" + m_defines.geometryShaderDefines +
                               geometryShaderText.substr(versionPos + 12);
        } else
            m_geometryShader = m_defines.geometryShaderDefines + geometryShaderText;
        if (processShader(ShaderType::GeometryShader, isHLSL) == false) return false;
    }

    return true;
}
/*!
    Shader constructor
    \param vertexShaderPath - path to vertex shader source
    \param fragmentShaderPath - path to fragment shader source
    \param geometryShaderPath - path to geometry shader source
    \throw std::bad_alloc - If an error occurs while allocating memory
    \todo add other shaders support
    */
Shader::Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath,
               const std::string_view geometryShaderPath, const ShaderDefines& defines) noexcept
    : m_vertShaderPath(vertexShaderPath), m_fragShaderPath(fragmentShaderPath),
      m_geometryShaderPath(geometryShaderPath), m_defines(defines) {
    m_isValid = compileShaders();
}

Shader::Shader(const Shader& other)
    : m_vertShaderPath(other.m_vertShaderPath),
      m_fragShaderPath(other.m_fragShaderPath),
      m_geometryShaderPath(other.m_geometryShaderPath),
      m_vertShader(other.m_vertShader),
      m_fragShader(other.m_fragShader),
      m_geometryShader(other.m_geometryShader),
      m_vertexShaderSpirV(other.m_vertexShaderSpirV),
      m_fragShaderSpirV(other.m_fragShaderSpirV),
      m_geometryShaderSpirV(other.m_geometryShaderSpirV),
      m_defines(other.m_defines) {}

Shader::Shader(Shader&& other) noexcept
    : m_vertShaderPath(std::move(other.m_vertShaderPath)),
      m_fragShaderPath(std::move(other.m_fragShaderPath)),
      m_geometryShaderPath(std::move(other.m_geometryShaderPath)),
      m_vertShader(std::move(other.m_vertShader)),
      m_fragShader(std::move(other.m_fragShader)),
      m_geometryShader(std::move(other.m_geometryShader)),
      m_vertexShaderSpirV(std::move(other.m_vertexShaderSpirV)),
      m_fragShaderSpirV(std::move(other.m_fragShaderSpirV)),
      m_geometryShaderSpirV(std::move(other.m_geometryShaderSpirV)),
      m_defines(std::move(other.m_defines)) {}

Shader& Shader::operator=(const Shader& other) {
    m_vertShaderPath = other.m_vertShaderPath;
    m_fragShaderPath = other.m_fragShaderPath;
    m_geometryShaderPath = other.m_geometryShaderPath;
    m_vertShader = other.m_vertShader;
    m_fragShader = other.m_fragShader;
    m_geometryShader = other.m_geometryShader;
    m_vertexShaderSpirV = other.m_vertexShaderSpirV;
    m_fragShaderSpirV = other.m_fragShaderSpirV;
    m_geometryShaderSpirV = other.m_geometryShaderSpirV;
    m_defines = other.m_defines;
    return *this;
}
Shader& Shader::operator=(Shader&& other) noexcept {
    m_vertShaderPath = std::move(other.m_vertShaderPath);
    m_fragShaderPath = std::move(other.m_fragShaderPath);
    m_geometryShaderPath = std::move(other.m_geometryShaderPath);
    m_vertShader = std::move(other.m_vertShader);
    m_fragShader = std::move(other.m_fragShader);
    m_geometryShader = std::move(other.m_geometryShader);
    m_vertexShaderSpirV = std::move(other.m_vertexShaderSpirV);
    m_fragShaderSpirV = std::move(other.m_fragShaderSpirV);
    m_geometryShaderSpirV = std::move(other.m_geometryShaderSpirV);
    m_defines = std::move(other.m_defines);
    return *this;
}
const std::vector<uint32_t>& Shader::getVertexShader() const noexcept {
    assert(!m_vertexShaderSpirV.empty());
    return m_vertexShaderSpirV;
}
const std::vector<uint32_t>& Shader::getFragmentShader() const noexcept {
    assert(!m_fragShaderSpirV.empty());
    return m_fragShaderSpirV;
}
const std::vector<uint32_t>& Shader::getGeometryShader() const noexcept {
    assert(!m_geometryShaderSpirV.empty());
    return m_geometryShaderSpirV;
}
const std::string& Shader::getVertexShaderPath() const noexcept { return m_vertShaderPath; }
const std::string& Shader::getFragmentShaderPath() const noexcept { return m_fragShaderPath; }
const std::string& Shader::getGeometryShaderPath() const noexcept { return m_geometryShaderPath; }
const ShaderDefines& Shader::getDefines() const noexcept { return m_defines; }
const bool Shader::isGeometryShaderPresent() const noexcept { return !m_geometryShaderPath.empty(); }
const bool Shader::isValid() const noexcept { return m_isValid; }
bool Shader::recompile() noexcept {
    m_isValid = compileShaders();
    assert(m_isValid);
    return m_isValid;
}
}  // namespace sge
