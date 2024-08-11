#include "WorkGraphShaderPass.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Shaders/DirectX12WorkGraphShaderPass.h"
#endif
namespace Engine
{

	WorkGraphShaderPass::WorkGraphShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		ShaderPass(src, passName), m_PassConfig(*src->config.FindWorkGraphPass(passName))
	{
		m_ComputeShader = true;
	}

	Ref<WorkGraphShaderPass> WorkGraphShaderPass::Create(Ref<ShaderSorce> src, const std::string& passName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12WorkGraphShaderPass>(src, passName);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

