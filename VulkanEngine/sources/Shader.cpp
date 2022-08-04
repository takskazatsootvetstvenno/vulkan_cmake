#pragma once
#include "Shader.h"
#include "Logger.h"
#include <fstream>
#include <cassert>
#include <shaderc/shaderc.hpp>

namespace sge {
	std::string Shader::readFile(const std::string_view filePath) noexcept
	{
		std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
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

	void Shader::processShader(const ShaderType type, bool isHLSL) noexcept
	{
		shaderc::CompileOptions compilerOptions;
		compilerOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
		compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		compilerOptions.SetTargetSpirv(shaderc_spirv_version_1_5);
		compilerOptions.SetWarningsAsErrors();
		compilerOptions.SetGenerateDebugInfo();

		const shaderc_source_language lang = isHLSL ? shaderc_source_language_hlsl : shaderc_source_language_glsl;
		compilerOptions.SetSourceLanguage(lang);
		shaderc::SpvCompilationResult result;
		switch (shaderc::Compiler compiler; type)
		{
		case sge::Shader::ShaderType::VertexShader:
			result = compiler.CompileGlslToSpv(m_vertShader, shaderc_vertex_shader, "sh.vert", "main", compilerOptions);
			m_vertexShaderSpirV = { result.cbegin(), result.cend() };
			break;
		case sge::Shader::ShaderType::FragmentShader:
			result = compiler.CompileGlslToSpv(m_fragShader, shaderc_fragment_shader, "sh.frag", "main", compilerOptions);
#if 0
			{auto preresult = compiler.PreprocessGlsl(m_fragShader, shaderc_fragment_shader, "sh.frag", compilerOptions);
			LOG_MSG(std::string{ preresult.begin(), preresult.end() });
		}
#endif 			
			m_fragShaderSpirV = { result.cbegin(), result.cend() };
			break;
		}
		if (result.GetCompilationStatus() != shaderc_compilation_status_success)
		{
			LOG_ERROR("Error message: " << result.GetErrorMessage());
			LOG_ERROR("Shader compile status: " << result.GetCompilationStatus());
			m_isValid = false;
			assert(false);
		}
	}

	Shader::Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath, const ShaderDefines& defines) noexcept
		:m_vertShaderPath(vertexShaderPath), m_fragShaderPath(fragmentShaderPath), m_defines(defines)
	{
		const auto vertexExtenstionPos = vertexShaderPath.rfind('.');
		if (vertexExtenstionPos == std::string::npos)
		{
			LOG_ERROR("Can't open shader file without extenstion!\nFile: " << vertexShaderPath);
			m_isValid = false;
			assert(false);
		}

		const auto fragmentExtenstionPos = fragmentShaderPath.rfind('.');
		if (fragmentExtenstionPos == std::string::npos)
		{
			LOG_ERROR("Can't open shader file without extenstion!\nFile: " << fragmentShaderPath);
			m_isValid = false;
			assert(false);
		}

		bool isHLSL = (vertexShaderPath.substr(vertexExtenstionPos) == ".hlsl");
		
		if (auto vertShaderText = readFile(m_vertShaderPath); !isHLSL) {
			auto versionPos = vertShaderText.find("#version ");
			m_vertShader = vertShaderText.substr(0, versionPos + 12) + "\n" + m_defines.vertShaderDefines  + vertShaderText.substr(versionPos + 12);
		}
		else
			m_vertShader = m_defines.vertShaderDefines + vertShaderText;

		processShader(ShaderType::VertexShader, isHLSL);

		isHLSL = (fragmentShaderPath.substr(fragmentExtenstionPos) == ".hlsl");

		if (auto fragShaderText = readFile(m_fragShaderPath); !isHLSL) {
			auto versionPos = fragShaderText.find("#version ");
			m_fragShader = fragShaderText.substr(0, versionPos + 12) + "\n" + m_defines.fragmentShaderDefines + fragShaderText.substr(versionPos + 12);
		}
		else
			m_fragShader = m_defines.fragmentShaderDefines + fragShaderText;

		processShader(ShaderType::FragmentShader, isHLSL);
	}
	Shader::Shader(const Shader& other)
		: m_vertShaderPath(other.m_vertShaderPath),
		m_fragShaderPath(other.m_fragShaderPath),
		m_vertShader(other.m_vertShader),
		m_fragShader(other.m_fragShader),
		m_vertexShaderSpirV(other.m_vertexShaderSpirV),
		m_fragShaderSpirV(other.m_fragShaderSpirV),
		m_defines(other.m_defines)
	{
	}
	Shader::Shader(Shader&& other) noexcept
		: m_vertShaderPath(std::move(other.m_vertShaderPath)),
		m_fragShaderPath(std::move(other.m_fragShaderPath)),
		m_vertShader(std::move(other.m_vertShader)),
		m_fragShader(std::move(other.m_fragShader)),
		m_vertexShaderSpirV(std::move(other.m_vertexShaderSpirV)),
		m_fragShaderSpirV(std::move(other.m_fragShaderSpirV)),
		m_defines(std::move(other.m_defines))
	{
	}
	Shader& Shader::operator=(const Shader& other)
	{
		m_vertShaderPath = other.m_vertShaderPath;
		m_fragShaderPath = other.m_fragShaderPath;
		m_vertShader = other.m_vertShader;
		m_fragShader = other.m_fragShader;
		m_vertexShaderSpirV = other.m_vertexShaderSpirV;
		m_fragShaderSpirV = other.m_fragShaderSpirV;
		m_defines = other.m_defines;
		return *this;
	}
	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_vertShaderPath = std::move(other.m_vertShaderPath);
		m_fragShaderPath = std::move(other.m_fragShaderPath);
		m_vertShader = std::move(other.m_vertShader);
		m_fragShader = std::move(other.m_fragShader);
		m_vertexShaderSpirV = std::move(other.m_vertexShaderSpirV);
		m_fragShaderSpirV = std::move(other.m_fragShaderSpirV);
		m_defines = std::move(other.m_defines);
		return *this;
	}
	const std::vector<uint32_t>& Shader::getVertexShader() const noexcept
	{
		assert(!m_vertexShaderSpirV.empty());
		return m_vertexShaderSpirV;
	}
	const std::vector<uint32_t>& Shader::getFragmentShader() const noexcept
	{
		assert(!m_fragShaderSpirV.empty());
		return m_fragShaderSpirV;
	}
	const std::string& Shader::getVertexShaderPath() const noexcept
	{
		return m_vertShaderPath;
	}
	const std::string& Shader::getFragmentShaderPath() const noexcept
	{
		return m_fragShaderPath;
	}
	const ShaderDefines& Shader::getDefines() const noexcept
	{
		return m_defines;
	}
	const bool Shader::isValid() const noexcept
	{
		return m_isValid;
	}
}
