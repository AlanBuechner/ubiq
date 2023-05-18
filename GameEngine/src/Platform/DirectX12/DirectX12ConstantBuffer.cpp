#include "pch.h"
#include "DirectX12ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12Context.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Util/Performance.h"


namespace Engine
{

	DirectX12ConstantBufferResource::DirectX12ConstantBufferResource(uint32 size)
	{
		size = size + 256 - (size % 256); // 256 byte aligned
		m_Size = size;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		// create resource
		HRESULT hr = context->GetDevice()->CreateCommittedResource(
			&props, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(m_Buffer.GetAddressOf())
		);
		CORE_ASSERT_HRESULT(hr, "Failed to create constant buffer resource");
	}

	DirectX12ConstantBufferResource::~DirectX12ConstantBufferResource()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_CBVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_CBVHandle);
		if (m_UAVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_UAVHandle);

		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12ConstantBufferResource::SetData(const void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, m_Size, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	void DirectX12ConstantBufferResource::CreateCBVHandle()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		m_CBVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = m_Buffer->GetGPUVirtualAddress();
		desc.SizeInBytes = m_Size;
		context->GetDevice()->CreateConstantBufferView(&desc, m_CBVHandle.cpu);
	}


	void DirectX12ConstantBufferResource::CreateUAVHandle()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		m_UAVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.CounterOffsetInBytes = 0;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = 1;
		uavDesc.Buffer.StructureByteStride = m_Size;
		context->GetDevice()->CreateUnorderedAccessView(m_Buffer.Get(), nullptr, &uavDesc, m_UAVHandle.cpu);
	}

	// Constant Buffer
	DirectX12ConstantBuffer::DirectX12ConstantBuffer(uint32 size)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		m_Resource = CreateRef<DirectX12ConstantBufferResource>(size);
		if(!m_Resource->m_CBVHandle) m_Resource->CreateCBVHandle();
	}

	DirectX12ConstantBuffer::DirectX12ConstantBuffer(Ref<ConstantBufferResource> resource)
	{
		m_Resource = std::dynamic_pointer_cast<DirectX12ConstantBufferResource>(resource);
		if (!m_Resource->m_CBVHandle) m_Resource->CreateCBVHandle();
	}

	// RW Constant Buffer
	DirectX12RWConstantBuffer::DirectX12RWConstantBuffer(uint32 size)
	{
		m_Resource = CreateRef<DirectX12ConstantBufferResource>(size);
		if(!m_Resource->m_UAVHandle) m_Resource->CreateUAVHandle();
	}

	DirectX12RWConstantBuffer::DirectX12RWConstantBuffer(Ref<ConstantBufferResource> resource)
	{
		m_Resource = std::dynamic_pointer_cast<DirectX12ConstantBufferResource>(resource);
		if (!m_Resource->m_UAVHandle) m_Resource->CreateUAVHandle();
	}

}
