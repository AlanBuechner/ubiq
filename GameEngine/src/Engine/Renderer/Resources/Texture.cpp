#include "pch.h"
#include "Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Texture.h"

namespace Engine
{
	Texture2DResource::~Texture2DResource() {}

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

	Ref<Texture2D> Texture2D::Create(const uint32 width, const uint32 height)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(width, height);
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
}
