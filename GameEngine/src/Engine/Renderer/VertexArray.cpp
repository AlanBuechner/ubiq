#include "pch.h"
#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"
#include "Renderer.h"

namespace Engine
{
	VertexArray * VertexArray::Create()
	{
		switch (Renderer::GetrAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::OpenGl:
			return new OpenGLVertexArray();
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}