#include "GraphicsShaderPass.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Shaders/DirectX12GraphicsShaderPass.h"
#endif

namespace Engine
{

	GraphicsShaderPass::GraphicsShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		ShaderPass(src, passName), m_PassConfig(*src->config.FindGraphicsPass(passName))
	{
		m_ComputeShader = false;
	}

	Ref<GraphicsShaderPass> GraphicsShaderPass::Create(Ref<ShaderSorce> src, const std::string& passName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12GraphicsShaderPass>(src, passName);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
