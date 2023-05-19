#include "pch.h"
#include "ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"

#include "Platform/DirectX12/DirectX12ConstantBUffer.h"

namespace Engine
{

	ConstantBufferResource::~ConstantBufferResource() {}

	bool ConstantBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case Engine::ShaderResource:
		case Engine::UnorderedResource:
			return true;
		case Engine::Common:
		case Engine::RenderTarget:
			return false;
		}
	}

	Ref<ConstantBuffer> ConstantBuffer::Create(uint32 size)
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

	Ref<ConstantBuffer> ConstantBuffer::Create(Ref<ConstantBufferResource> resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return std::make_shared<DirectX12ConstantBuffer>(resource);
		default:
			break;
		}
		return Ref<ConstantBuffer>();
	}


	// -------------- RWConstantBuffer -------------- //
	Ref<RWConstantBuffer> RWConstantBuffer::Create(uint32 size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return std::make_shared<DirectX12RWConstantBuffer>(size);
		default:
			break;
		}
		return Ref<RWConstantBuffer>();
	}

	Ref<RWConstantBuffer> RWConstantBuffer::Create(Ref<ConstantBufferResource> resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return std::make_shared<DirectX12RWConstantBuffer>(resource);
		default:
			break;
		}
		return Ref<RWConstantBuffer>();
	}

}
