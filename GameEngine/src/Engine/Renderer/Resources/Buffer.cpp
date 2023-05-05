#include "pch.h"
#include "Engine/Core/Core.h"
#include "Buffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Buffer.h"

namespace Engine
{

	uint32 ShaderDataTypeSize(ShaderDataType type)
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
		default: break;
		}
		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}


	BufferElement::BufferElement(ShaderDataType type, const std::string& name, bool normalized /*= false*/) : 
		Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
	{}

	uint32 BufferElement::GetComponentCount()
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
		default: break;
		}
		CORE_ASSERT(false, "Unknown ShaderDataType!");
		return 0;
	}

	BufferLayout::BufferLayout(const std::vector<BufferElement>& elements) :
		m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	BufferLayout::BufferLayout(const std::initializer_list<BufferElement>& elements) :
		m_Elements(elements)
	{
		CalculateOffsetsAndStride();
	}

	void BufferLayout::CalculateOffsetsAndStride()
	{
		uint32 offset = 0;
		m_Stride = 0;
		for (auto& element : m_Elements)
		{
			element.Offset = offset;
			offset += element.Size;
		}
		m_Stride = offset;
	}







	Ref<VertexBuffer> VertexBuffer::Create(uint32 count, uint32 stride)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12VertexBuffer>(count, stride);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32 count, uint32 stride)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12VertexBuffer>(vertices, count, stride);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12IndexBuffer>(count);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}

	Ref<IndexBuffer> IndexBuffer::Create(const uint32* indices, uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::None:
			CORE_ASSERT(false, "RendererAPI::None is currently not supported!");
			return nullptr;
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12IndexBuffer>(indices, count);
		}
		CORE_ASSERT(false, "Unknown RendererAPI!");
		return nullptr;
	}
}
