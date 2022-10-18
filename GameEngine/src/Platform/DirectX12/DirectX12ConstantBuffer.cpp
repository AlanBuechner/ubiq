#include "pch.h"
#include "DirectX12ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12Context.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Util/Performance.h"


namespace Engine
{
	DirectX12ConstantBuffer::DirectX12ConstantBuffer(uint32 size) :
		m_Size(size)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		// create resource
		HRESULT hr = context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(size), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(m_Buffer.GetAddressOf())
		);
		CORE_ASSERT_HRESULT(hr, "Failed to create constant buffer resource");

		// create DescriptorHandle
		m_Handle = DirectX12ResourceManager::s_SRVHeap->Allocate();

	}

	DirectX12ConstantBuffer::~DirectX12ConstantBuffer()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_Handle)
			m_Handle.Free();

		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12ConstantBuffer::SetData(void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, m_Size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

}
