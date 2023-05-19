#include "pch.h"
#include "Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Texture.h"

namespace Engine
{
	bool IsDepthStencil(TextureFormat format)
	{
		switch (format)
		{
		case Engine::TextureFormat::DEPTH24STENCIL8:
			return true;
		}
		return false;
	}


	Texture2DResource::~Texture2DResource() {}


	uint32 Texture2DResource::GetStride()
	{
		switch (m_Format)
		{
		case TextureFormat::RGBA8:
			return 4*1;
		case TextureFormat::RGBA16:
			return 4*2;
		case TextureFormat::RGBA32:
			return 4*4;
		case TextureFormat::RG16:
			return 2*2;
		case TextureFormat::RG32:
			return 2*4;
		case TextureFormat::RED_INTEGER:
			return 4*1;
		case TextureFormat::DEPTH24STENCIL8:
			return 24+8;
		default:
			break;
		}
		return 0;
	}

	bool Texture2DResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::Common:
		case ResourceState::ShaderResource:
		case ResourceState::UnorderedResource:
		case ResourceState::RenderTarget:
			return true;
		}
	}

	Ref<Texture2D> Texture2D::Create(const fs::path& path)
	{
		CORE_ASSERT(path != "" ,"Path must be given");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(path);
		}
		return Ref<Texture2D>();
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height)
	{
		return Create(width, height, 0, TextureFormat::RGBA8);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, uint32 mips)
	{
		return Create(width, height, mips, TextureFormat::RGBA8);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, TextureFormat format)
	{
		return Create(width, height, 0, format);
	}

	Ref<Texture2D> Texture2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(width, height, mips, format);
		}
		return Ref<Texture2D>();
	}

	Ref<Texture2D> Texture2D::Create(Ref<Texture2DResource> resource)
	{
		CORE_ASSERT(resource, "width and height cant be 0");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(resource);
		}
		return Ref<Texture2D>();
	}

	bool Texture2D::ValidExtension(const fs::path& ext)
	{
		return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp");
	}

	void RenderTarget2D::Resize(uint32 width, uint32 height)
	{

	}

	Ref<RenderTarget2D> RenderTarget2D::Create(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12RenderTarget2D>(width, height, mips, format);
		}
		return Ref<DirectX12RenderTarget2D>();
	}

	Ref<RenderTarget2D> RenderTarget2D::Create(Ref<Texture2DResource> resource)
	{
		CORE_ASSERT(resource, "width and height cant be 0");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12RenderTarget2D>(resource);
		}
		return Ref<DirectX12RenderTarget2D>();
	}

}
