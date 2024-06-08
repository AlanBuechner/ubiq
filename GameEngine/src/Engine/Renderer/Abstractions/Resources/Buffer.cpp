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
		default: return nullptr;
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
		default: return false;
		}
	}




	VertexBufferView* VertexBufferView::Create(VertexBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			VertexBufferView* handle = new DirectX12VertexBufferView();
			handle->Bind(resource);
			return handle;
		}
		default: return nullptr;
		}
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
		default: return nullptr;
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
		default: return false;
		}
	}




	IndexBufferView* IndexBufferView::Create(IndexBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			IndexBufferView* handle = new DirectX12IndexBufferView();
			handle->Bind(resource);
			return handle;
		}
		default: return nullptr;
		}
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
