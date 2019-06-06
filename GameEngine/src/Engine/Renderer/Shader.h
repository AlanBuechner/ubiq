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

		enum ShaderType
		{
			None = -1, Vertex = BIT(0), Pixle = BIT(1)
		};

	public:
		virtual ~Shader() {};

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		static ShaderSorce LoadShader(std::string file);
		static ShaderSorce LoadShader(std::string file, int type);

		static Shader* Create(const std::string& vertexSrc, const std::string& pixleSrc);
	};
}