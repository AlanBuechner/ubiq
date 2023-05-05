#include "pch.h"
#include "StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12StructuredBuffer.h"

namespace Engine
{

	Ref<StructuredBuffer> StructuredBuffer::Create(uint32 stride, uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12StructuredBuffer>(stride, count);
		default:
			break;
		}
		return Ref<StructuredBuffer>();
	}

}

