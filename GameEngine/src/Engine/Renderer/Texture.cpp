#include "pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Engine
{
	Ref<Texture2D> Texture2D::Create(const std::string& path, TextureAttribute& attribute)
	{
		CORE_ASSERT(path != "" ,"Path must be given");
		
		Ref<TextureAttribute> attrib = std::make_shared<TextureAttribute>(attribute);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGl:
			return std::make_shared<OpenGLTexture2D>(path, attrib);
		}
		
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
	Ref<Texture2D> Texture2D::Create(const uint32_t width, const uint32_t height, TextureAttribute& attribute)
	{
		CORE_ASSERT(width != 0 && height != 0, "width and height cant be 0");

		Ref<TextureAttribute> attrib = std::make_shared<TextureAttribute>(attribute);

		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGl:
			return std::make_shared<OpenGLTexture2D>(width, height, attrib);
		}

		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}