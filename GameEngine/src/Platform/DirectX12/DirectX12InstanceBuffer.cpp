#include "pch.h"
#include "DirectX12InstanceBuffer.h"
#include "Directx12Context.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	DirectX12InstanceBuffer::DirectX12InstanceBuffer(uint32 stride, uint32 count) : 
		m_Stride(stride)
	{
		CreateUploadBuffer(count);
	}

	DirectX12InstanceBuffer::~DirectX12InstanceBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_UploadBuffer);
		CD3DX12_RANGE range = CD3DX12_RANGE(0, 0);
		m_UploadBuffer->Unmap(0, &range);
		m_MapLoc = nullptr;
	}

	void DirectX12InstanceBuffer::SetData(uint32 start, uint32 count, const void* data)
	{
		uint32 newSize = start + count;
		if (newSize > m_Capacity)
			CreateUploadBuffer(newSize*2);
		if (newSize > m_UploadCount)
			m_UploadCount = newSize;
		
		// copy the buffer
		void* startLoc = (void*)((uint64)m_MapLoc + (start * m_Stride));
		memcpy(startLoc, data, count * m_Stride);
	}

	void DirectX12InstanceBuffer::PushBack(uint32 count, const void* data)
	{
		SetData(m_UploadCount, count, data);
	}

	void DirectX12InstanceBuffer::Apply()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_Count != m_Capacity)
			CreateBuffer(m_Capacity);
		context->GetDX12ResourceManager()->CopyBuffer(m_Buffer, m_UploadBuffer, m_UploadCount*m_Stride, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void* DirectX12InstanceBuffer::At(uint32 index)
	{
		return (void*)((uint64)m_MapLoc + (index * m_Stride));
	}

	void DirectX12InstanceBuffer::CreateBuffer(uint32 count)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_Buffer)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(count * m_Stride);

		context->GetDevice()->CreateCommittedResource(
			&props, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);

		m_View.BufferLocation = m_Buffer->GetGPUVirtualAddress();
		m_View.StrideInBytes = m_Stride;
		m_View.SizeInBytes = count * m_Stride;

		m_Count = count;
	}

	void DirectX12InstanceBuffer::CreateUploadBuffer(uint32 count)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		wrl::ComPtr<ID3D12Resource> oldUploadBuffer = m_UploadBuffer;
		void* oldMapLoc = m_MapLoc;
		// create new buffer
		m_UploadBuffer = DirectX12ResourceManager::CreateUploadBuffer(count*m_Stride);
		m_MapLoc = nullptr; // reset map loc
		m_UploadBuffer->Map(0, nullptr, &m_MapLoc); // the cpu may read from this buffer

		// copy data from old buffer
		if (oldUploadBuffer)
		{
			memcpy(m_MapLoc, oldMapLoc, 0);
			
			// clean up old buffer
			CD3DX12_RANGE range = CD3DX12_RANGE(0, 0);
			oldUploadBuffer->Unmap(0, &range);
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(oldUploadBuffer);
		}

		m_Capacity = count;
	}

}
