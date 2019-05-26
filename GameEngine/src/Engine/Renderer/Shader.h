#pragma once
#include <string>

namespace Engine
{
	class Shader
	{
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind();
		void UnBind();

		static bool LoadShader(std::string file, std::string& src);

	private:
		uint32_t m_RendererID;
	};
}