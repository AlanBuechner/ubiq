#include "pch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLShader.h"

#include <fstream>

namespace Engine
{

	Shader::ShaderSorce Shader::LoadShader(const fs::path& file)
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
				else if (line.find("pixel") != std::string::npos || line.find("fragment") != std::string::npos)
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

		sorce.vertexPath = file;
		sorce.pixlePath = file;

		return sorce;
	}

	Shader::ShaderSorce Shader::LoadShader(const fs::path& file, int type)
	{
		ShaderSorce src = LoadShader(file);
		ShaderSorce srcToReturn;
		if ((type & (int)ShaderType::Vertex) != 0)
			srcToReturn.vertexShader = src.vertexShader;
		if ((type & (int)ShaderType::Pixle) != 0)
			srcToReturn.pixleShader = src.pixleShader;

		return srcToReturn;
	}

	void Shader::ReloadShader(ShaderSorce& shaders)
	{
		// TODO : add new Reload Shader Function
	}

	Ref<Shader> Shader::Create(const std::string& name, Ref<Shader::ShaderSorce> src)
	{
		CORE_ASSERT(name != "", "Shader must have a name");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGl:
			return CreateRef<OpenGLShader>(name, src);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!")
			return nullptr;
	}

	void ShaderLibrary::Init()
	{
		switch (RendererAPI::GetAPI())
		{
		case RendererAPI::API::OpenGl:
			break;
		default:
			break;
		}
	}

	void ShaderLibrary::Add(const Ref<Shader>& shader)
	{
		auto& name = shader->GetName();
		CORE_ASSERT(m_Shaders.find(name) == m_Shaders.end(), "Shader name allredy taken");
		m_Shaders[name] = shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& path)
	{
		Ref<Shader::ShaderSorce> src = CreateRef<Shader::ShaderSorce>();
		*src.get() << Shader::LoadShader(path);
		auto shader = Shader::Create(name, src);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Load(const std::string& name, Ref<Shader::ShaderSorce> src)
	{
		auto shader = Shader::Create(name, src);
		Add(shader);
		return shader;
	}

	Ref<Shader> ShaderLibrary::Get(const std::string& name)
	{
		CORE_ASSERT(m_Shaders.find(name) != m_Shaders.end(), "Shader not found");
		return m_Shaders[name];
	}
}
