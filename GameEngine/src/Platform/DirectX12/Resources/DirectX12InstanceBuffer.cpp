#include "pch.h"
#include "DirectX12InstanceBuffer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Renderer/Renderer.h"

#include "DirectX12UploadBuffer.h"

namespace Engine
{

	DirectX12InstanceBufferResource::DirectX12InstanceBufferResource(uint32 capacity, uint32 stride)
	{
		m_DefultState = ResourceState::ShaderResource;
		m_Stride = stride;
		m_Capacity = capacity;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_Capacity * m_Stride);

		context->GetDevice()->CreateCommittedResource(
			&props, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);

		m_UploadBuffer = UploadBufferResource::Create(m_Capacity * m_Stride);
		m_UploadBuffer->Map();
	}

	DirectX12InstanceBufferResource::~DirectX12InstanceBufferResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
		m_UploadBuffer->UnMap();
		delete m_UploadBuffer;
		m_UploadBuffer = nullptr;
	}

	void DirectX12InstanceBufferResource::SetData(const void* data, uint32 count)
	{		
		// copy the buffer
		memcpy(m_UploadBuffer->GetMemory(), data, count * m_Stride);

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->CopyBuffer(this, m_UploadBuffer, count * m_Stride, m_DefultState);
	}



	uint32 DirectX12InstanceBufferResource::GetGPUState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		default: return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	void DirectX12InstanceBufferView::Bind(InstanceBufferResource* resource)
	{
		DirectX12InstanceBufferResource* dxResource = (DirectX12InstanceBufferResource*)resource;

		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.StrideInBytes = dxResource->GetStride();
		m_View.SizeInBytes = dxResource->GetStride() * dxResource->GetCapacity();
	}

}
