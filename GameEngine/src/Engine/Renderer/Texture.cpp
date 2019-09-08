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
}