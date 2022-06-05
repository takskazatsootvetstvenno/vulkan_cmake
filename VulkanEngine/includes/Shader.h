#pragma once
#include <string>
#include <vector>
#include <string_view>
namespace sge {
	class Shader
	{
	public:
		Shader(const std::string_view vertexShaderPath, const std::string_view fragmentShaderPath) noexcept;
		Shader(const Shader&) = delete;
		Shader(Shader&& other) noexcept;
		Shader& operator=(const Shader&) = delete;
		Shader& operator=(Shader&& other) noexcept;

		[[nodiscard]] const std::vector<uint32_t>& getVertexShader() const noexcept;
		[[nodiscard]] const std::vector<uint32_t>& getFragmentShader() const noexcept;
		[[nodiscard]] const std::string& getVertexShaderPath() const noexcept;
		[[nodiscard]] const std::string& getFragmentShaderPath() const noexcept;
		[[nodiscard]] const bool isValid() const noexcept;

	private:
		enum class ShaderType
		{
			VertexShader = 0,
			FragmentShader
		};
		std::string readFile(const std::string_view filePath) noexcept;
		void processShader(const std::string& source, ShaderType type, bool isHLSL = false);
		std::string m_vertShaderPath;
		std::string m_fragShaderPath;
		std::string m_vertShader;
		std::string m_fragShader;
		//std::string m_vertexShaderSpirV;
		//std::string m_fragShaderSpirV;
		std::vector<uint32_t> m_vertexShaderSpirV;
		std::vector<uint32_t> m_fragShaderSpirV;
		bool m_isValid = true;
	};
}