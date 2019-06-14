#include "pch.h"
#include "Engine/core.h"
#include "Engine/Log.h"
#include "Buffer.h"
#include "Renderer.h"
#include "Platform/OpenGL/OpenGLBuffer.h"

namespace Engine
{

	uint32_t ShaderDataTypeSize(ShaderDataType type)
	{
		switch (type)
		{
		case Engine::ShaderDataType::Float:		return 4;
		case Engine::ShaderDataType::Float2:	return 4 * 2;
		case Engine::ShaderDataType::Float3:	return 4 * 3;
		case Engine::ShaderDataType::Float4:	return 4 * 4;
		case Engine::ShaderDataType::Mat3:		return 4 * 3 * 3;
		case Engine::ShaderDataType::Mat4:		return 4 * 4 * 4;
		case Engine::ShaderDataType::Int:		return 4;
		case Engine::ShaderDataType::Int2:		return 4 * 2;
		case Engine::ShaderDataType::Int3:		return 4 * 3;
		case Engine::ShaderDataType::Int4:		return 4 * 4;
		case Engine::ShaderDataType::Bool:		return 1;
		}
		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	inline uint32_t BufferElement::GetComponentCount()
	{
		switch (Type)
		{
		case Engine::ShaderDataType::Float:		return 1;
		case Engine::ShaderDataType::Float2:	return 2;
		case Engine::ShaderDataType::Float3:	return 3;
		case Engine::ShaderDataType::Float4:	return 4;
		case Engine::ShaderDataType::Mat3:		return 3 * 3;
		case Engine::ShaderDataType::Mat4:		return 4 * 4;
		case Engine::ShaderDataType::Int:		return 1;
		case Engine::ShaderDataType::Int2:		return 2;
		case Engine::ShaderDataType::Int3:		return 3;
		case Engine::ShaderDataType::Int4:		return 4;
		case Engine::ShaderDataType::Bool:		return 1;
		}
		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	inline void BufferLayout::CalculateOffsetsAndStride()
	{
		uint32_t offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
		}
		m_Stride = offset;
	}




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