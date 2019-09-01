#include "pch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>

namespace Engine
{

	Shader::ShaderSorce Shader::LoadShader(std::string file)
	{
		std::ifstream shaderFile;
		shaderFile.open(file);
		if (shaderFile.fail())
		{
			CORE_ERROR("Cant open file {0}", file);
			return ShaderSorce();
		}

		enum class Type
		{
			NONE = -1, VERTEX = 0, PIXLE = 1
		};

		Type mode = Type::NONE;

		ShaderSorce sorce;

		std::stringstream ss[2];

		std::string line;
		while (getline(shaderFile, line))
		{
			if (line.find("#shader") != std::string::npos)
			{
				if (line.find("vertex") != std::string::npos) 
					mode = Type::VERTEX;
				else if (line.find("pixle") != std::string::npos || line.find("fragment") != std::string::npos)
					mode = Type::PIXLE;
			}
			else
			{
				ss[(int)mode] << line << '\n';
			}
		}

		shaderFile.close();

		sorce.vertexShader = ss[(int)Type::VERTEX].str();
		sorce.pixleShader = ss[(int)Type::PIXLE].str();

		sorce.path.push_back(std::pair(file, ShaderType::None));

		return sorce;
	}

	Shader::ShaderSorce Shader::LoadShader(std::string file, int type)
	{
		ShaderSorce src = LoadShader(file);
		ShaderSorce srcToReturn;
		if ((type & (int)ShaderType::Vertex) != 0)
			srcToReturn.vertexShader = src.vertexShader;
		if ((type & (int)ShaderType::Pixle) != 0)
			srcToReturn.pixleShader = src.pixleShader;

		srcToReturn.path.push_back(std::pair(file, type));

		return srcToReturn;
	}

	void Shader::ReloadShader(ShaderSorce& shaders)
	{
		for (int i = 0; i < shaders.path.size(); i++)
		{
			auto [path, type] = shaders.path[i];
			if (type == ShaderType::None)
			{
				shaders << LoadShader(path);
			}
			else
			{
				shaders << LoadShader(path, type);
			}
		}
	}

	Ref<Shader> Shader::Create(Ref<Shader::ShaderSorce> src)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGl:
			return std::make_shared<OpenGLShader>(src);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!")
			return nullptr;
	}
}