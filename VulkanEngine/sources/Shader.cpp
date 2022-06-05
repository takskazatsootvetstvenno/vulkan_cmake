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

	void Shader::processShader(const std::string& source, const ShaderType type, const bool isHLSL)
	{
		shaderc::Compiler compiler;
		shaderc::CompileOptions compilerOptions;
		compilerOptions.SetOptimizationLevel(shaderc_optimization_level_performance);
		compilerOptions.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
		compilerOptions.SetTargetSpirv(shaderc_spirv_version_1_5);
		compilerOptions.SetWarningsAsErrors();
		const shaderc_source_language lang = isHLSL ? shaderc_source_language_hlsl : shaderc_source_language_glsl;
		compilerOptions.SetSourceLanguage(lang);
		shaderc::SpvCompilationResult result;
		switch (type)
		{
		case sge::Shader::ShaderType::VertexShader:
			result = compiler.CompileGlslToSpv(m_vertShader, shaderc_vertex_shader, "sh.vert", "main", compilerOptions);
			m_vertexShaderSpirV = { result.cbegin(), result.cend() };
			break;
		case sge::Shader::ShaderType::FragmentShader:
			result = compiler.CompileGlslToSpv(m_fragShader, shaderc_fragment_shader, "sh.frag", "main", compilerOptions);
			m_fragShaderSpirV = { result.cbegin(), result.cend() };
			break;
		}

		if (result.GetCompilationStatus())
		{
			LOG_ERROR("Error message: " << result.GetErrorMessage());
			LOG_ERROR("Shader compile status: " << result.GetCompilationStatus())
			assert(false);
		}
	}

	Shader::Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath) noexcept
		:m_vertShaderPath(vertexShaderPath), m_fragShaderPath(fragmentShaderPath)
	{
		const auto vertexExtenstionPos = vertexShaderPath.rfind('.');
		if (vertexExtenstionPos == std::string::npos)
		{
			LOG_ERROR("Can't open shader file without extenstion!\nFile: " << vertexShaderPath);
			assert(false);
		}

		const auto fragmentExtenstionPos = fragmentShaderPath.rfind('.');
		if (fragmentExtenstionPos == std::string::npos)
		{
			LOG_ERROR("Can't open shader file without extenstion!\nFile: " << fragmentShaderPath);
			assert(false);
		}

		m_vertShader = readFile(m_vertShaderPath);
		m_fragShader = readFile(m_fragShaderPath);

		bool isHLSL = (vertexShaderPath.substr(vertexExtenstionPos) == ".hlsl");
		processShader(m_vertShader, ShaderType::VertexShader, isHLSL);

		isHLSL = (fragmentShaderPath.substr(fragmentExtenstionPos) == ".hlsl");
		processShader(m_fragShader, ShaderType::FragmentShader, isHLSL);
	}
	Shader::Shader(Shader&& other) noexcept
		: m_vertShaderPath(std::move(other.m_vertShaderPath)),
		m_fragShaderPath(std::move(other.m_fragShaderPath)),
		m_vertShader(std::move(other.m_vertShader)),
		m_fragShader(std::move(other.m_fragShader)),
		m_vertexShaderSpirV(std::move(other.m_vertexShaderSpirV)),
		m_fragShaderSpirV(std::move(other.m_fragShaderSpirV))
	{
	}
	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_vertShaderPath = std::move(other.m_vertShaderPath);
		m_fragShaderPath = std::move(other.m_fragShaderPath);
		m_vertShader = std::move(other.m_vertShader);
		m_fragShader = std::move(other.m_fragShader);
		m_vertexShaderSpirV = std::move(other.m_vertexShaderSpirV);
		m_fragShaderSpirV = std::move(other.m_fragShaderSpirV);
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
	const bool Shader::isValid() const noexcept
	{
		return m_isValid;
	}
}
