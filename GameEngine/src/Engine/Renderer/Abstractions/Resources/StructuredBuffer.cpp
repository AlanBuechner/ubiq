#include "pch.h"
#include "StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12StructuredBuffer.h"
#endif

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
		case ResourceState::CopySource:
		case ResourceState::CopyDestination:
			return true;
		case ResourceState::PiplineInput:
		{
			if (m_ParentType == ParentType::StructuredBuffer) return false;
			else return true;
		}
		default: return false;
		}
	}

	StructuredBufferResource* StructuredBufferResource::Create(uint32 count, uint32 stride, ParentType parentType)
	{
		CREATE_PROFILE_SCOPEI("Create Structured Buffer Resource");
		ANOTATE_PROFILEI(std::to_string(count * stride) + " bytes");
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12StructuredBufferResource(count, stride, parentType);
		default: return nullptr;
		}
	}

	// Descriptor Handles ---------------------------------------------------------- //

	StructuredBufferSRVDescriptorHandle* StructuredBufferSRVDescriptorHandle::Create(StructuredBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			StructuredBufferSRVDescriptorHandle* handle = new DirectX12StructuredBufferSRVDescriptorHandle();
			handle->Bind(resource);
			return handle;
		}
		default: return nullptr;
		}
	}


	StructuredBufferUAVDescriptorHandle* StructuredBufferUAVDescriptorHandle::Create(StructuredBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			StructuredBufferUAVDescriptorHandle* handle = new DirectX12StructuredBufferUAVDescriptorHandle();
			handle->Bind(resource);
			return handle;
		}
		default: return nullptr;
		}
	}




	// Structured Buffer Objects ---------------------------------------------------------- //

	StructuredBuffer::StructuredBuffer(uint32 count, uint32 stride, StructuredBufferResource::ParentType parantType)
	{
		m_Resource = StructuredBufferResource::Create(count, stride, parantType);
		m_SRVDescriptor = StructuredBufferSRVDescriptorHandle::Create(m_Resource);

	}

	StructuredBuffer::StructuredBuffer(uint32 count, uint32 stride) :
		StructuredBuffer(count, stride, StructuredBufferResource::ParentType::StructuredBuffer)
	{}

	StructuredBuffer::StructuredBuffer(StructuredBufferResource* resource, StructuredBufferSRVDescriptorHandle* srv)
	{
		m_DataOwner = false;
		m_Resource = resource;
		m_SRVDescriptor = srv;
	}

	StructuredBuffer::~StructuredBuffer()
	{
		if (m_DataOwner)
		{
			Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		}
		m_Resource = nullptr;
		m_SRVDescriptor = nullptr;
	}

	void StructuredBuffer::Resize(uint32 count)
	{
		// schedule old resource destruction
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);

		// create new resource
		m_Resource = StructuredBufferResource::Create(count, m_Resource->GetStride(), m_Resource->GetParentType());

		// rebind srv handles
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_SRVDescriptor);
		m_SRVDescriptor = StructuredBufferSRVDescriptorHandle::Create(m_Resource);
	}

	Ref<StructuredBuffer> StructuredBuffer::Create(uint32 count, uint32 stride)
	{
		return CreateRef<StructuredBuffer>(count, stride);
	}


	RWStructuredBuffer::RWStructuredBuffer(StructuredBufferResource* resource, StructuredBufferSRVDescriptorHandle* srv, StructuredBuffer* owner) :
		StructuredBuffer(resource, srv), m_Owner(owner)
	{
		GenerateUAVDescriptor();
	}

	RWStructuredBuffer::RWStructuredBuffer(uint32 count, uint32 stride) :
		StructuredBuffer(count, stride)
	{
		GenerateUAVDescriptor();
	}

	RWStructuredBuffer::~RWStructuredBuffer()
	{
		StructuredBuffer::~StructuredBuffer();
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptor);
	}

	void RWStructuredBuffer::Resize(uint32 count)
	{
		if (m_Owner)
			m_Owner->Resize(count);
		else
		{
			StructuredBuffer::Resize(count);
			GenerateUAVDescriptor();
		}
	}

	void RWStructuredBuffer::GenerateUAVDescriptor()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_UAVDescriptor);
		m_UAVDescriptor = StructuredBufferUAVDescriptorHandle::Create(m_Resource);
	}

	Ref<RWStructuredBuffer> RWStructuredBuffer::Create(uint32 count, uint32 stride)
	{
		return CreateRef<RWStructuredBuffer>(count, stride);
	}
}

