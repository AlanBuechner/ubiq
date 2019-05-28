#pragma once
#include <string>
#include "Engine/core.h"

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

		enum class ShaderType
		{
			NONE = -1, VERTEX = BIT(0), PIXLE = BIT(1)
		};
	public:
		Shader(const std::string& vertexSrc, const std::string& fragmentSrc);
		~Shader();

		void Bind();
		void UnBind();

		static ShaderSorce LoadShader(std::string file);
		static ShaderSorce LoadShader(std::string file, ShaderType type);

	protected:

	private:
		uint32_t m_RendererID;
	};
}