#include "pch.h"
#include "SwapChain.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12SwapChain.h"
#endif

namespace Engine
{
	Ref<SwapChain> Engine::SwapChain::Create(Window& window)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12SwapChain>(window);
		default:
			break;
		}
		return Ref<SwapChain>();
	}

}
