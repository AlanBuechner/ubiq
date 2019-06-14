#include "pch.h"
#include "Engine/core.h"
#include "Engine/Log.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Engine
{
	VertexBuffer* VertexBuffer::Create(float* vertices, uint32_t size)
	{
		switch (Renderer::GetrAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::OpenGl:
			return new OpenGLVertexBuffer(vertices, size);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	IndexBuffer* IndexBuffer::Create(uint32_t* indices, uint32_t size)
	{
		switch (Renderer::GetrAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::OpenGl:
			return new OpenGLIndexBuffer(indices, size);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}