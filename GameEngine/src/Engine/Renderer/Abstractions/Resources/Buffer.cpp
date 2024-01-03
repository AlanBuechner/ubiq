#include "pch.h"
#include "Buffer.h"
#include "Engine/Renderer/Abstractions/Resources/ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/Resources/DirectX12Buffer.h"

namespace Engine
{

	// VertexBuffer -------------------------------------------------------------------------------------

	VertexBufferResource::~VertexBufferResource() {}

	VertexBufferResource* VertexBufferResource::Create(uint32 count, uint32 stride)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12VertexBufferResource(count, stride);
		}
	}

	bool VertexBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		}
		return false;
	}




	VertexBufferView* VertexBufferView::Create(VertexBufferResource* resource)
	{
		VertexBufferView* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12VertexBufferView();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}



	VertexBuffer::VertexBuffer(uint32 count, uint32 stride)
	{
		m_Resource = VertexBufferResource::Create(count, stride);
		m_View = VertexBufferView::Create(m_Resource);
	}

	VertexBuffer::~VertexBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);
	}

	void VertexBuffer::Resize(uint32 count)
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);

		m_Resource = VertexBufferResource::Create(count, m_Resource->GetStride());
		m_View = VertexBufferView::Create(m_Resource);
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32 count, uint32 stride)
	{
		return CreateRef<VertexBuffer>(count, stride);
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32 count, uint32 stride)
	{
		Ref<VertexBuffer> buffer = Create(count, stride);
		buffer->SetData(vertices);
		return buffer;
	}


	// IndexBuffer ---------------------------------------------------------------------------------------

	IndexBufferResource::~IndexBufferResource() {}

	IndexBufferResource* IndexBufferResource::Create(uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12IndexBufferResource(count);
		}
	}

	bool IndexBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		}
		return false;
	}




	IndexBufferView* IndexBufferView::Create(IndexBufferResource* resource)
	{
		IndexBufferView* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12IndexBufferView();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}



	IndexBuffer::IndexBuffer(uint32 count)
	{
		m_Resource = IndexBufferResource::Create(count);
		m_View = IndexBufferView::Create(m_Resource);
	}

	IndexBuffer::~IndexBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);
	}

	void IndexBuffer::Resize(uint32 count)
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);

		m_Resource = IndexBufferResource::Create(count);
		m_View = IndexBufferView::Create(m_Resource);
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32 count)
	{
		return CreateRef<IndexBuffer>(count);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const void* vertices, uint32 count)
	{
		Ref<IndexBuffer> buffer = Create(count);
		buffer->SetData(vertices);
		return buffer;
	}
}
