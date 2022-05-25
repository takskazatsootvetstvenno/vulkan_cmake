#pragma once
#include "Shader.h"
#include "Logger.h"
#include <fstream>
#include <cassert>

namespace sge {
	std::string Shader::readFile(const std::string_view filePath) noexcept
	{
		std::ifstream file(filePath.data(), std::ios::ate | std::ios::binary);
		if (!file.is_open())
		{
			LOG_ERROR("Failed to open file: " << filePath << "!")
			assert(false);
		}
		const size_t fileSize = static_cast<size_t>(file.tellg());
		std::string buffer;
		buffer.resize(fileSize);
		file.seekg(0);
		file.read(buffer.data(), fileSize);
		file.close();
		return buffer;
	}

	Shader::Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath) noexcept
		:m_vertShaderPath(vertexShaderPath),m_fragShaderPath(fragmentShaderPath)
	{
		m_vertShader = readFile(m_vertShaderPath);
		m_fragShader = readFile(m_fragShaderPath);
	}
	Shader::Shader(Shader&& other) noexcept
		: m_vertShaderPath(std::move(other.m_vertShaderPath)),
		m_fragShaderPath(std::move(other.m_fragShaderPath)),
		m_vertShader(std::move(other.m_vertShader)),
		m_fragShader(std::move(other.m_fragShader))
	{
	}
	Shader& Shader::operator=(Shader&& other) noexcept
	{
		m_vertShaderPath = std::move(other.m_vertShaderPath);
		m_fragShaderPath = std::move(other.m_fragShaderPath);
		m_vertShader = std::move(other.m_vertShader);
		m_fragShader = std::move(other.m_fragShader);
		return *this;
	}
	const std::string& Shader::getVertexShader() const noexcept
	{
		assert(!m_vertShader.empty());
		return m_vertShader;
	}
	const std::string& Shader::getFragmentShader() const noexcept
	{
		assert(!m_fragShader.empty());
		return m_fragShader;
	}
	const std::string& Shader::getVertexShaderPath() const noexcept
	{
		return m_vertShaderPath;
	}
	const std::string& Shader::getFragmentShaderPath() const noexcept
	{
		return m_fragShaderPath;
	}
}