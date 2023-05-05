#include "pch.h"
#include "Texture.h"

#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Texture.h"

namespace Engine
{
	Ref<Texture2D> Texture2D::Create(const fs::path& path, TextureAttribute& attribute)
	{
		CORE_ASSERT(path != "" ,"Path must be given");
		
		Ref<TextureAttribute> attrib = CreateRef<TextureAttribute>(attribute);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(path, attrib);
		}
		
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<Texture2D> Texture2D::Create(const uint32 width, const uint32 height, TextureAttribute& attribute)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");

		Ref<TextureAttribute> attrib = CreateRef<TextureAttribute>(attribute);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12Texture2D>(width, height, attrib);
		}

		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	bool Texture2D::ValidExtention(const fs::path& ext)
	{
		return (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp");
	}
}
