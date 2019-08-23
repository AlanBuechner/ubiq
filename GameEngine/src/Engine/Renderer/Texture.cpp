#include "pch.h"
#include "Texture.h"

#include "Renderer.h"
#include "Platform/OpenGL/OpenGLTexture.h"

namespace Engine
{
	Ref<Texture2D> Texture2D::Create(const std::string& path)
	{
		if (path == "")
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::None:
				CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGl:
				return std::make_shared<OpenGLTexture2D>();
			}
		}
		else
		{
			switch (Renderer::GetAPI())
			{
			case RendererAPI::API::None:
				CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
				return nullptr;
			case RendererAPI::API::OpenGl:
				return std::make_shared<OpenGLTexture2D>(path);
			}
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}