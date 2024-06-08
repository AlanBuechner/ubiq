#include "UploadBuffer.h"
#include "Platform/DirectX12/Resources/DirectX12UploadBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/ResourceManager.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{
	UploadBufferResource::~UploadBufferResource() {}

	UploadBufferResource* UploadBufferResource::Create(uint32 size)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12UploadBufferResource(size);
		default: return nullptr;
		}
	}

	bool UploadBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		default: return false;
		}
	}



	UploadTextureResource::~UploadTextureResource() {}

	UploadTextureResource* UploadTextureResource::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12UploadTextureResource(width, height, mips, format);
		default: return nullptr;
		}
	}

	bool UploadTextureResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		default: return false;
		}
	}
}

