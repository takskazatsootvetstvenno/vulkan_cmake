#pragma once
#include <string>
#include <vector>
#include <string_view>

namespace sge {

	struct ShaderDefines
	{
		std::string vertShaderDefines;
		std::string fragmentShaderDefines;
		std::string geometryShaderDefines;
	};
	class Shader
	{
	public:
		explicit Shader(
			const std::string_view vertexShaderPath,
			const std::string_view fragmentShaderPath,
			const std::string_view geometryShaderPath = "",
			const ShaderDefines& defines = ShaderDefines{}) noexcept;

		Shader(const Shader& other);
		Shader(Shader&& other) noexcept;
		Shader& operator=(const Shader& other);
		Shader& operator=(Shader&& other) noexcept;

		[[nodiscard]] const std::vector<uint32_t>& getVertexShader() const noexcept;
		[[nodiscard]] const std::vector<uint32_t>& getFragmentShader() const noexcept;
		[[nodiscard]] const std::vector<uint32_t>& getGeometryShader() const noexcept;
		[[nodiscard]] const std::string& getVertexShaderPath() const noexcept;
		[[nodiscard]] const std::string& getFragmentShaderPath() const noexcept;
		[[nodiscard]] const std::string& getGeometryShaderPath() const noexcept;
		[[nodiscard]] const ShaderDefines& getDefines() const noexcept;
		[[nodiscard]] const bool isGeometryShaderPresent() const noexcept;
		[[nodiscard]] const bool isValid() const noexcept;
		bool recompile() noexcept;

	private:
		enum class ShaderType
		{
			VertexShader = 0,
			FragmentShader = 1,
			GeometryShader = 2
		};
		std::string readFile(const std::string_view filePath) noexcept;
		bool processShader(const ShaderType type, bool isHLSL = false) noexcept;
		bool compileShaders() noexcept;
		std::string m_vertShaderPath;
		std::string m_fragShaderPath;
		std::string m_geometryShaderPath;
		std::string m_vertShader;
		std::string m_fragShader;
		std::string m_geometryShader;
		std::vector<uint32_t> m_vertexShaderSpirV;
		std::vector<uint32_t> m_fragShaderSpirV;
		std::vector<uint32_t> m_geometryShaderSpirV;
		ShaderDefines m_defines;
		bool m_isValid = true;
		bool m_lastSuccessful = false;
	};
}