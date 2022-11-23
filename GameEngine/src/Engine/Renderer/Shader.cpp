#include "pch.h"
#include "Shader.h"
#include "Renderer.h"
#include "Platform/DirectX12/DirectX12Shader.h"
#include "Platform/DirectX12/DirectX12ComputeShader.h"

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
		return CreateRef<Shader>(file);
	}

	Ref<ComputeShader> ComputeShader::Create(const fs::path& file)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12ComputeShader>(file);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}
