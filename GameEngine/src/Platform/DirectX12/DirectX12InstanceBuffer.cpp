#include "pch.h"
#include "DirectX12InstanceBuffer.h"
#include "Directx12Context.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	DirectX12InstanceBuffer::DirectX12InstanceBuffer(uint32 stride, uint32 count) : 
		m_Stride(stride), m_Count(count)
	{
		CreateBuffer();
	}

	DirectX12InstanceBuffer::~DirectX12InstanceBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12InstanceBuffer::SetData(uint32 start, uint32 count, const void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		
		// copy the buffer
		context->GetDX12ResourceManager()->UploadBufferRegion(m_Buffer, start * m_Stride, data, count * m_Stride, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void DirectX12InstanceBuffer::CreateBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_Buffer)
			m_Buffer->Release();

		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(m_Count * m_Stride), // resource description for a buffer
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);

		m_View.BufferLocation = m_Buffer->GetGPUVirtualAddress();
		m_View.StrideInBytes = m_Stride;
		m_View.SizeInBytes = m_Count * m_Stride;
	}

}
