#include "pch.h"
#include "InstanceBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "ResourceManager.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12InstanceBuffer.h"
#endif

namespace Engine
{

	InstanceBufferResource::~InstanceBufferResource(){}

	InstanceBufferResource* InstanceBufferResource::Create(uint32 capacity, uint32 stride)
	{
		CREATE_PROFILE_SCOPEI("Create Instance Buffer Resource");
		ANOTATE_PROFILEI(std::to_string(capacity * stride) + " bytes");

		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return new DirectX12InstanceBufferResource(capacity, stride);
		default: return nullptr;
		}
	}

	bool InstanceBufferResource::SupportState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::CopyDestination:
		case ResourceState::ShaderResource:
			return true;
		default: return false;
		}
	}

	InstanceBufferView* InstanceBufferView::Create(InstanceBufferResource* resource)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
		{
			InstanceBufferView* handle = new DirectX12InstanceBufferView();
			handle->Bind(resource);
			return handle;
		} 
		default: return nullptr;
		}
	}






	InstanceBuffer::InstanceBuffer(uint32 capacity, uint32 stride)
	{
		m_Stride = stride;
		m_Count = 0;
		m_Capacity = pow(2, ceil(log2(capacity)));

		m_Resource = InstanceBufferResource::Create(m_Capacity, stride);
		m_View = InstanceBufferView::Create(m_Resource);

		m_Data = malloc(m_Capacity * m_Stride);
	}

	InstanceBuffer::~InstanceBuffer()
	{
		Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
		Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);

		free(m_Data);
		m_Data = nullptr;
	}

	void InstanceBuffer::PushBack(const void* data, uint32 count)
	{
		uint32 newCount = m_Count + count;
		if (newCount >= m_Capacity)
		{
			void* oldData = m_Data;
			m_Capacity = pow(2, ceil(log2(newCount)));
			m_Data = malloc(m_Capacity * m_Stride);
			memcpy(m_Data, oldData, m_Count * m_Stride);
			free(oldData);
		}

		void* startLoc = (void*)((uint64)m_Data + (m_Count * m_Stride));
		memcpy(startLoc, data, count * m_Stride);

		m_Count = newCount;
	}

	void InstanceBuffer::Apply()
	{
		if (m_Resource->GetCapacity() != m_Capacity)
		{
			Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_Resource);
			Renderer::GetContext()->GetResourceManager()->ScheduleHandleDeletion(m_View);
			m_Resource = InstanceBufferResource::Create(m_Stride, m_Capacity);
			m_View = InstanceBufferView::Create(m_Resource);
		}
		m_Resource->SetData(m_Data, m_Count);
	}

	Ref<InstanceBuffer> InstanceBuffer::Create(uint32 capacity, uint32 stride)
	{
		return CreateRef<InstanceBuffer>(capacity, stride);
	}

}
