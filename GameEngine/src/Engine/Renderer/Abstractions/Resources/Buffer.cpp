#include "pch.h"
#include "Buffer.h"
#include "Engine/Renderer/Abstractions/Resources/ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/Resources/DirectX12Buffer.h"

namespace Engine
{

	// VertexBuffer -------------------------------------------------------------------------------------


	VertexBufferView* VertexBufferView::Create(StructuredBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			VertexBufferView* handle = new DirectX12VertexBufferView(resource);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
	}



	VertexBuffer::VertexBuffer(uint32 count, uint32 stride, bool RWCapable) :
		StructuredBuffer(count, stride, StructuredBufferResource::ParentType::VertexBuffer)
	{
		m_View = VertexBufferView::Create(m_Resource);
		if(RWCapable)
			m_RWStructuredBuffer = CreateRef<RWStructuredBuffer>(m_Resource, m_SRVDescriptor, this);
	}

	VertexBuffer::~VertexBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);
		m_View = nullptr;

		m_RWStructuredBuffer.reset();
	}

	void VertexBuffer::Resize(uint32 count)
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);

		StructuredBuffer::Resize(count);

		m_View = VertexBufferView::Create(m_Resource);

		if (m_RWStructuredBuffer)
		{
			m_RWStructuredBuffer->m_Resource = m_Resource;
			m_RWStructuredBuffer->m_SRVDescriptor = m_SRVDescriptor;
			m_RWStructuredBuffer->GenerateUAVDescriptor();
		}
	}

	Ref<VertexBuffer> VertexBuffer::Create(uint32 count, uint32 stride, bool RWCapable)
	{
		return CreateRef<VertexBuffer>(count, stride, RWCapable);
	}

	Ref<VertexBuffer> VertexBuffer::Create(const void* vertices, uint32 count, uint32 stride, bool RWCapable)
	{
		Ref<VertexBuffer> buffer = Create(count, stride, RWCapable);
		buffer->SetData(vertices, count);
		return buffer;
	}


	// IndexBuffer ---------------------------------------------------------------------------------------



	IndexBufferView* IndexBufferView::Create(StructuredBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			IndexBufferView* handle = new DirectX12IndexBufferView(resource);
			handle->Bind();
			return handle;
		}
		default: return nullptr;
		}
	}



	IndexBuffer::IndexBuffer(uint32 count, bool RWCapable) :
		StructuredBuffer(count, sizeof(uint32), StructuredBufferResource::ParentType::IndexBuffer)
	{
		m_View = IndexBufferView::Create(m_Resource);

		if (RWCapable)
			m_RWStructuredBuffer = CreateRef<RWStructuredBuffer>(m_Resource, m_SRVDescriptor, this);
	}

	IndexBuffer::~IndexBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);
		m_View = nullptr;

		m_RWStructuredBuffer.reset();
	}

	void IndexBuffer::Resize(uint32 count)
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);

		StructuredBuffer::Resize(count);
		m_View = IndexBufferView::Create(m_Resource);

		if (m_RWStructuredBuffer)
		{
			m_RWStructuredBuffer->m_Resource = m_Resource;
			m_RWStructuredBuffer->m_SRVDescriptor = m_SRVDescriptor;
			m_RWStructuredBuffer->GenerateUAVDescriptor();
		}
	}

	Ref<IndexBuffer> IndexBuffer::Create(uint32 count, bool RWCapable)
	{
		return CreateRef<IndexBuffer>(count, RWCapable);
	}

	Ref<IndexBuffer> IndexBuffer::Create(const void* vertices, uint32 count, bool RWCapable)
	{
		Ref<IndexBuffer> buffer = Create(count, RWCapable);
		buffer->SetData(vertices, count);
		return buffer;
	}
}
