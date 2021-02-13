#include "pch.h"
#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Engine
{
	Ref<VertexArray> VertexArray::Create()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::API::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::API::OpenGl:
			return std::make_shared<OpenGLVertexArray>();
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}