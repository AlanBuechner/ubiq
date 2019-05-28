#pragma once
#include <string>

namespace Engine
{
	class Shader
	{
	public:
		struct ShaderSorce
		{
			std::string vertexShader;
			std::string pixleShader;

			void operator<<(const ShaderSorce& other)
			{
				if (this->vertexShader.empty()) this->vertexShader = other.vertexShader;
				if (this->pixleShader.empty()) this->pixleShader = other.pixleShader;
			}
		};
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind();
		void UnBind();

		static ShaderSorce LoadShader(std::string file);

	protected:

	private:
		uint32_t m_RendererID;
	};
}