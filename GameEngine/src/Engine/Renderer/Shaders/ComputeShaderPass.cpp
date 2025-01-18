#include "ComputeShaderPass.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Core/Logging/Log.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Shaders/DirectX12ComputeShaderPass.h"
#endif
namespace Engine
{

	ComputeShaderPass::ComputeShaderPass(Ref<ShaderSorce> src, const std::string& passName) :
		ShaderPass(src, passName), m_PassConfig(*src->config.FindComputePass(passName))
	{
		m_ComputeShader = true;
	}

	Ref<ComputeShaderPass> ComputeShaderPass::Create(Ref<ShaderSorce> src, const std::string& passName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12ComputeShaderPass>(src, passName);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

}

