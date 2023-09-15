#include "pch.h"
#include "StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12StructuredBuffer.h"

namespace Engine
{

	// Structured Buffer Resource ---------------------------------------------------------- //

	StructuredBufferResource::~StructuredBufferResource() {}

	bool StructuredBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
		case ResourceState::UnorderedResource:
			return true;
		case ResourceState::Common:
		case ResourceState::RenderTarget:
			return false;
		}
	}

	StructuredBufferResource* StructuredBufferResource::Create(uint32 stride, uint32 count)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12StructuredBufferResource(stride, count);
		}
		return nullptr;
	}

	// Descriptor Handles ---------------------------------------------------------- //

	StructuredBufferSRVDescriptorHandle* StructuredBufferSRVDescriptorHandle::Create(StructuredBufferResource* resource)
	{
		StructuredBufferSRVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12StructuredBufferSRVDescriptorHandle();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}


	StructuredBufferUAVDescriptorHandle* StructuredBufferUAVDescriptorHandle::Create(StructuredBufferResource* resource)
	{
		StructuredBufferUAVDescriptorHandle* handle = nullptr;
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			handle = new DirectX12StructuredBufferUAVDescriptorHandle();
			break;
		}

		if (handle)
			handle->Bind(resource);
		return handle;
	}




	// Structured Buffer Objects ---------------------------------------------------------- //

	StructuredBuffer::StructuredBuffer(uint32 stride, uint32 count)
	{
		m_Resource = StructuredBufferResource::Create(stride, count);
		m_SRVDescriptor = StructuredBufferSRVDescriptorHandle::Create(m_Resource);
	}

	StructuredBuffer::~StructuredBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
	}

	void StructuredBuffer::Resize(uint32 count)
	{
		// schedule old resource destruction
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);

		// create new resource
		m_Resource = StructuredBufferResource::Create(m_Resource->GetStride(), count);

		// rebind srv handles
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		m_SRVDescriptor = StructuredBufferSRVDescriptorHandle::Create(m_Resource);
	}

	Ref<StructuredBuffer> StructuredBuffer::Create(uint32 stride, uint32 count)
	{
		return CreateRef<StructuredBuffer>(stride, count);
	}


	RWStructuredBuffer::RWStructuredBuffer(uint32 stride, uint32 count) :
		StructuredBuffer(stride, count)
	{
		m_UAVDescriptor = StructuredBufferUAVDescriptorHandle::Create(m_Resource);
	}

	RWStructuredBuffer::~RWStructuredBuffer()
	{
		StructuredBuffer::~StructuredBuffer();
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptor);
	}

	void RWStructuredBuffer::Resize(uint32 count)
	{
		StructuredBuffer::Resize(count);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptor);
		m_UAVDescriptor = StructuredBufferUAVDescriptorHandle::Create(m_Resource);
	}

	Ref<RWStructuredBuffer> RWStructuredBuffer::Create(uint32 stride, uint32 count)
	{
		return CreateRef<RWStructuredBuffer>(stride, count);
	}
}

