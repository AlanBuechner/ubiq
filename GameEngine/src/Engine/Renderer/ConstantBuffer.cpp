#include "pch.h"
#include "ConstantBuffer.h"
#include "Renderer.h"

#include "Platform/DirectX12/DirectX12ConstantBUffer.h"

namespace Engine
{
	Ref<ConstantBuffer> Engine::ConstantBuffer::Create(uint32 size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return std::make_shared<DirectX12ConstantBuffer>(size);
		default:
			break;
		}
		return Ref<ConstantBuffer>();
	}

}
