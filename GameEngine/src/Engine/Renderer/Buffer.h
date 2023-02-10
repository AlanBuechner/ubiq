#pragma once
#include "Engine/Core/Core.h"

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

	[[maybe_unused]]static uint32 ShaderDataTypeSize(ShaderDataType type);

	struct BufferElement
	{
		std::string Name;
		ShaderDataType Type;
		uint32 Size;
		uint32 Offset;
		bool Normalized;

		BufferElement(){}
		BufferElement(ShaderDataType type, const std::string& name, bool normalized = false);

		uint32 GetComponentCount();
	};

	class BufferLayout
	{
	public:
		BufferLayout(){}
		BufferLayout(const std::vector<BufferElement>& elements);
		BufferLayout(const std::initializer_list<BufferElement>& elements);

		inline const std::vector<BufferElement>& GetElements() const { return m_Elements; }
		inline const uint32 GetStride() const { return m_Stride; }

		std::vector<BufferElement>::iterator begin() { return m_Elements.begin(); }
		std::vector<BufferElement>::iterator end() { return m_Elements.end(); }
		std::vector<BufferElement>::const_iterator begin() const { return m_Elements.begin(); }
		std::vector<BufferElement>::const_iterator end() const { return m_Elements.end(); }
	private:
		void CalculateOffsetsAndStride();
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
