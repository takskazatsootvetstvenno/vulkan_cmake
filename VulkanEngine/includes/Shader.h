#pragma once
#include<string>
#include<string_view>
namespace sge {
	class Shader
	{
	public:
		Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath) noexcept;
		Shader(const Shader&) = delete;
		Shader(Shader&& other) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& other) noexcept;

		[[nodiscard]] const std::string& getVertexShader() const noexcept;
		[[nodiscard]] const std::string& getFragmentShader() const noexcept;
		[[nodiscard]] const std::string& getVertexShaderPath() const noexcept;
		[[nodiscard]] const std::string& getFragmentShaderPath() const noexcept;
	private:
		std::string readFile(const std::string_view filePath) noexcept;

		std::string m_vertShaderPath;
		std::string m_fragShaderPath;
		std::string m_vertShader;
		std::string m_fragShader;
	};
}