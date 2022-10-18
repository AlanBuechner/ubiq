#include "pch.h"
#include "FrameBuffer.h"

#include "Engine/Renderer/Renderer.h"

#include "Platform/DirectX12/DirectX12FrameBuffer.h"

namespace Engine
{
	Ref<FrameBuffer> FrameBuffer::Create(const FrameBufferSpecification& spec)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12FrameBuffer>(spec);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
