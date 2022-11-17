#include "pch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/DirectX12/DirectX12Shader.h"

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

	Shader::Shader(const fs::path& file)
	{
		Ref<ShaderSorce> src = ShaderCompiler::LoadFile(file);

		for (const ShaderConfig::RenderPass& pass : src->config.passes)
		{
			m_Passes[pass.passName] = ShaderPass::Create(src, pass.passName);
		}
	}

	Ref<ShaderPass> Shader::GetPass(const std::string& passName)
	{
		return m_Passes[passName];
	}

	Ref<Shader> Shader::Create(const fs::path& file)
	{
		return CreateRef<Shader>(file);
	}

}
