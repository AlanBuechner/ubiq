#include "pch.h"
#include "Renderer.h"
#include "GraphicsContext.h"
#include "Platform/DirectX12/Directx12Context.h"

namespace Engine
{
	Ref<GraphicsContext> Engine::GraphicsContext::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Context>();
		default:
			break;
		}
		return Ref<GraphicsContext>();
	}

}
