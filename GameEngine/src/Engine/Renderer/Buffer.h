#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Log.h"

namespace Engine
{
	enum class ShaderDataType
	{
		None = 0, 
		Float, Float2, Float3, Float4, 
		Mat3, Mat4,
		Int, Int2, Int3, Int4,
		Bool
	};

	static uint32 ShaderDataTypeSize(ShaderDataType type)
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

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32 Size;
		uint32 Offset;
		bool Normalized;

		BufferElement(){}
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false)
			: Name(name), Type(type), Size(ShaderDataTypeSize(type)), Offset(0), Normalized(normalized)
		{}

		uint32 GetComponentCount()
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
	};

	class BufferLayout
	{
	public:
		BufferLayout(){}
		BufferLayout(const std::vector<BufferElement>& elements) :
			m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}
		BufferLayout(const std::initializer_list<BufferElement>& elements) :
			m_Elements(elements)
		{
			CalculateOffsetsAndStride();
		}

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline const uint32 GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride()
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
	private:
		std::vector<BufferElement> m_Elements;
		uint32 m_Stride = 0;
	};

	class VertexBuffer
	{
	public:
		virtual ~VertexBuffer() {}

		virtual void SetLayout(const BufferLayout& layout) = 0;
		virtual BufferLayout& GetLayout() = 0;

		virtual void SetData(const void* data, uint32 size) = 0;

		static Ref<VertexBuffer> Create(uint32 count, uint32 stride);
		static Ref<VertexBuffer> Create(const void* vertices, uint32 count, uint32 stride);
	};

	class IndexBuffer
	{
	public:
		virtual ~IndexBuffer() {}

		virtual uint32 GetCount() = 0;

		virtual void SetData(const uint32* data, uint32 count) = 0;

		static Ref<IndexBuffer> Create(uint32 count);
		static Ref<IndexBuffer> Create(const uint32* indices, uint32 count);
	};
}
