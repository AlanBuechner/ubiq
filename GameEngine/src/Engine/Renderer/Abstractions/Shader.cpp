#include "pch.h"
#include "Shader.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Shader.h"

#include "Engine/Util/PlatformUtils.h"
#include "EngineResource.h"


#include <fstream>

namespace Engine
{
	Ref<ShaderPass> ShaderPass::Create(Ref<ShaderSorce> src, const std::string& passName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Shader>(src, passName);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Shader::Shader(const std::string& code, const fs::path& file)
	{
		std::stringstream is(code);
		Ref<ShaderSorce> src = ShaderCompiler::LoadFromSrc(is, file);

		for (const ShaderConfig::RenderPass& pass : src->config.passes)
		{
			m_Passes[pass.passName] = ShaderPass::Create(src, pass.passName);
		}

		m_Params = src->config.params;
	}

	Ref<ShaderPass> Shader::GetPass(const std::string& passName)
	{
		auto pass = m_Passes.find(passName);
		if (pass == m_Passes.end())
			return nullptr;
		return pass->second;
	}

	std::vector<MaterialParameter>& Shader::GetParams()
	{
		return m_Params;
	}

	Ref<Shader> Shader::Create(const fs::path& file)
	{
		std::ifstream ifs(file);
		if (ifs.fail())
		{
			CORE_ERROR("Cant open file {0}", file.string());
			return nullptr;
		}

		std::string str((std::istreambuf_iterator<char>(ifs)), std::istreambuf_iterator<char>());
		return CreateFromSrc(str, file);
	}

	Ref<Shader> Shader::CreateFromEmbeded(uint32 id, const fs::path& file)
	{
		uint32 size = 0;
		byte* data = nullptr;

		if (GetEmbededResource(SHADER, id, data, size))
		{
			std::string src;
			src.assign((char*)data, size);

			return CreateFromSrc(src, file);
		}
		else
		{
			CORE_ERROR("Could not load shader from embeded: \"{0}\"", file.string());
			return nullptr;
		}
	}

	Ref<Shader> Shader::CreateFromSrc(const std::string& src, const fs::path& file /*= ""*/)
	{
		return CreateRef<Shader>(src, file);
	}

}
