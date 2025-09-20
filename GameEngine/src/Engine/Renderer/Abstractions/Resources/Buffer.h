#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"
#include "StructuredBuffer.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------


	// vertex buffers don't use descriptors they use views but architecturally they are similar
	class VertexBufferView : public Descriptor
	{
	protected:
		VertexBufferView(StructuredBufferResource* resource) :
			m_Resource(resource)
		{ }

	public:

		static VertexBufferView* Create(StructuredBufferResource* resource);

	protected:
		StructuredBufferResource* m_Resource;
	};

	class VertexBuffer : public StructuredBuffer
	{
	public:
		VertexBuffer(uint32 count, uint32 stride, bool RWCapable = false);
		~VertexBuffer();

		VertexBufferView* GetView() { return m_View; }
		bool IsRWCapable() { return m_RWStructuredBuffer != nullptr; }
		Ref<RWStructuredBuffer> GetRWStructuredBuffer() { return m_RWStructuredBuffer; }

		void Resize(uint32 count);

		static Ref<VertexBuffer> Create(uint32 count, uint32 stride, bool RWCapable = false);
		static Ref<VertexBuffer> Create(const void* vertices, uint32 count, uint32 stride, bool RWCapable = false);

	protected:
		VertexBufferView* m_View;
		Ref<RWStructuredBuffer> m_RWStructuredBuffer;
	};




	// IndexBuffer ---------------------------------------------------------------------------------------


	// index buffers don't use descriptors they use views but architecturally they are similar
	class IndexBufferView : public Descriptor
	{
	protected:
		IndexBufferView(StructuredBufferResource* resource) :
			m_Resource(resource)
		{ }

	public:

		static IndexBufferView* Create(StructuredBufferResource* resource);

	protected:
		StructuredBufferResource* m_Resource;
	};

	class IndexBuffer : public StructuredBuffer
	{
	public:
		IndexBuffer(uint32 count, bool RWCapable = false);
		~IndexBuffer();

		IndexBufferView* GetView() { return m_View; }
		Ref<RWStructuredBuffer> GetRWStructuredBuffer() { return m_RWStructuredBuffer; }

		void Resize(uint32 count);

		static Ref<IndexBuffer> Create(uint32 count, bool RWCapable = false);
		static Ref<IndexBuffer> Create(const void* indices, uint32 count, bool RWCapable = false);

	protected:
		IndexBufferView* m_View;
		Ref<RWStructuredBuffer> m_RWStructuredBuffer;
	};
}
