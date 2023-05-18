#include "pch.h"
#include "StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12StructuredBuffer.h"

namespace Engine
{

	StructuredBufferResource::~StructuredBufferResource() {}

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

	Ref<StructuredBuffer> StructuredBuffer::Create(Ref<StructuredBufferResource> resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12StructuredBuffer>(resource);
		default:
			break;
		}
		return Ref<StructuredBuffer>();
	}


	Ref<RWStructuredBuffer> RWStructuredBuffer::Create(uint32 stride, uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12RWStructuredBuffer>(stride, count);
		default:
			break;
		}
		return Ref<RWStructuredBuffer>();
	}

	Ref<RWStructuredBuffer> RWStructuredBuffer::Create(Ref<StructuredBufferResource> resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12RWStructuredBuffer>(resource);
		default:
			break;
		}
		return Ref<RWStructuredBuffer>();
	}

}

