#pragma once
#include "Engine/Core/Core.h"
#include "ResourceState.h"
#include "Descriptor.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	class VertexBufferResource : public GPUResource
	{
	public:
		virtual ~VertexBufferResource() = 0;

		uint32 GetCount() { return m_Count; }
		uint32 GetStride() { return m_Stride; }

		virtual void SetData(const void* data) = 0;

		static VertexBufferResource* Create(uint32 count, uint32 stride);

	protected:
		bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Count;
		uint32 m_Stride;

	};

	// vertex buffers don't use descriptors they use views but architecturally they are similar
	class VertexBufferView : public Descriptor
	{
	public:
		virtual void Bind(VertexBufferResource* resource) = 0;

		static VertexBufferView* Create(VertexBufferResource* resource);
	};

	class VertexBuffer
	{
	public:
		VertexBuffer(uint32 count, uint32 stride);
		~VertexBuffer();

		VertexBufferResource* GetResource() { return m_Resource; }
		VertexBufferView* GetView() { return m_View; }

		void Resize(uint32 count);

		void SetData(const void* data) { m_Resource->SetData(data); }

		static Ref<VertexBuffer> Create(uint32 count, uint32 stride);
		static Ref<VertexBuffer> Create(const void* vertices, uint32 count, uint32 stride);

	protected:
		VertexBufferResource* m_Resource;
		VertexBufferView* m_View;
	};




	// IndexBuffer ---------------------------------------------------------------------------------------

	class IndexBufferResource : public GPUResource
	{
	public:
		virtual ~IndexBufferResource() = 0;

		uint32 GetCount() { return m_Count; }

		virtual void SetData(const void* data) = 0;

		static IndexBufferResource* Create(uint32 count);

	protected:
		bool SupportState(ResourceState state) override;

	protected:
		uint32 m_Count;

	};

	// index buffers don't use descriptors they use views but architecturally they are similar
	class IndexBufferView : public Descriptor
	{
	public:
		virtual void Bind(IndexBufferResource* resource) = 0;

		static IndexBufferView* Create(IndexBufferResource* resource);
	};

	class IndexBuffer
	{
	public:
		IndexBuffer(uint32 count);
		~IndexBuffer();

		IndexBufferResource* GetResource() { return m_Resource; }
		IndexBufferView* GetView() { return m_View; }

		void Resize(uint32 count);

		void SetData(const void* data) { m_Resource->SetData(data); }

		static Ref<IndexBuffer> Create(uint32 count);
		static Ref<IndexBuffer> Create(const void* indices, uint32 count);

	protected:
		IndexBufferResource* m_Resource;
		IndexBufferView* m_View;
	};
}
