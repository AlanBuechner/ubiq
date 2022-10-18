#include "pch.h"
#include "SwapChain.h"
#include "Renderer.h"

#include "Platform/DirectX12/DirectX12SwapChain.h"

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
