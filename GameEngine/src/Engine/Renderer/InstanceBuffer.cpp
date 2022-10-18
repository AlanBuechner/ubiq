#include "pch.h"
#include "InstanceBuffer.h"
#include "Renderer.h"

#include "Platform/DirectX12/DirectX12InstanceBuffer.h"

namespace Engine
{

	Ref<InstanceBuffer> InstanceBuffer::Create(uint32 stride, uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return std::make_shared<DirectX12InstanceBuffer>(stride, count);
		default:
			break;
		}
		return Ref<InstanceBuffer>();
	}

}
