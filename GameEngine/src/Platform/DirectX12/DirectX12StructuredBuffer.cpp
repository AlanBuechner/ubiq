#include "pch.h"
#include "DirectX12StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12Context.h"

namespace Engine
{

	DirectX12StructuredBufferResource::DirectX12StructuredBufferResource(uint32 stride, uint32 count)
	{
		m_Stride = stride;
		Resize(count);
	}

	DirectX12StructuredBufferResource::~DirectX12StructuredBufferResource()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_SRVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_SRVHandle);
		if (m_UAVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_UAVHandle);

		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12StructuredBufferResource::Resize(uint32 count)
	{
		m_Count = count;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_Stride * count);

		if (m_Buffer)
			context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);

		context->GetDevice()->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE,
			nullptr,
			IID_PPV_ARGS(m_Buffer.ReleaseAndGetAddressOf())
		);

		m_Buffer->SetName(L"Structured Buffer");

		// recreate used handles
		if (m_SRVHandle) CreateSRVHandle();
		if (m_UAVHandle) CreateUAVHandle();
	}

	void DirectX12StructuredBufferResource::SetData(const void* data, uint32 count /*= 1*/, uint32 start /*= 0*/)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBufferRegion(m_Buffer, m_Stride * start, data, m_Stride * count, 
			D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	void DirectX12StructuredBufferResource::CreateSRVHandle()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_SRVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_SRVHandle);

		m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = m_Count;
		srvDesc.Buffer.StructureByteStride = m_Stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		context->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_SRVHandle.cpu);
	}

	void DirectX12StructuredBufferResource::CreateUAVHandle()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		if (m_UAVHandle) context->GetDX12ResourceManager()->ScheduleHandleDeletion(m_UAVHandle);

		m_UAVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void* DirectX12StructuredBufferResource::GetGPUResourcePointer()
	{
		return (void*)m_Buffer.Get();
	}

	uint32 DirectX12StructuredBufferResource::GetState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case ResourceState::UnorderedResource:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		}
	}




	DirectX12StructuredBuffer::DirectX12StructuredBuffer(uint32 stride, uint32 count)
	{
		m_Resource = CreateRef<DirectX12StructuredBufferResource>(stride, count);
		if(!m_Resource->m_SRVHandle) m_Resource->CreateSRVHandle();
	}

	DirectX12StructuredBuffer::DirectX12StructuredBuffer(Ref<StructuredBufferResource> resource)
	{
		m_Resource = std::dynamic_pointer_cast<DirectX12StructuredBufferResource>(resource);
		if (!m_Resource->m_SRVHandle) m_Resource->CreateSRVHandle();
	}




	DirectX12RWStructuredBuffer::DirectX12RWStructuredBuffer(uint32 stride, uint32 count)
	{
		m_Resource = CreateRef<DirectX12StructuredBufferResource>(stride, count);
		if(!m_Resource->m_UAVHandle) m_Resource->CreateUAVHandle();
	}

	DirectX12RWStructuredBuffer::DirectX12RWStructuredBuffer(Ref<StructuredBufferResource> resource)
	{
		m_Resource = std::dynamic_pointer_cast<DirectX12StructuredBufferResource>(resource);
		if (!m_Resource->m_UAVHandle) m_Resource->CreateUAVHandle();
	}

}

