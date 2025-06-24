#include "pch.h"
#include "DirectX12StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"

namespace Engine
{

	DirectX12StructuredBufferResource::DirectX12StructuredBufferResource(uint32 count, uint32 stride, ParentType parentType)
	{
		if (parentType == StructuredBufferResource::ParentType::StructuredBuffer)
			m_DefultState = ResourceState::ShaderResource;
		else
			m_DefultState = ResourceState::PiplineInput;
		m_Count = count;
		m_Stride = stride;
		m_ParentType = parentType;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(m_Stride * count);
		resDesc.Flags |= D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;

		context->GetDevice()->CreateCommittedResource(
			&props,
			D3D12_HEAP_FLAG_NONE,
			&resDesc,
			(D3D12_RESOURCE_STATES)GetGPUState(m_DefultState),
			nullptr,
			IID_PPV_ARGS(&m_Buffer)
		);

		m_Buffer->SetName(L"Structured Buffer");
	}

	DirectX12StructuredBufferResource::~DirectX12StructuredBufferResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void DirectX12StructuredBufferResource::SetData(const void* data, uint32 count, uint32 start)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBufferRegion(this, m_Stride * start, data, m_Stride * count, m_DefultState);
	}

	uint32 DirectX12StructuredBufferResource::GetGPUState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE;
		case ResourceState::UnorderedResource:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		case ResourceState::PiplineInput:
		{
			switch (m_ParentType)
			{
			case StructuredBufferResource::ParentType::VertexBuffer:
				return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
			case StructuredBufferResource::ParentType::IndexBuffer:
				return D3D12_RESOURCE_STATE_INDEX_BUFFER;
			default: return D3D12_RESOURCE_STATE_COMMON;
			}
		}
		default: return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	DirectX12StructuredBufferSRVDescriptorHandle::DirectX12StructuredBufferSRVDescriptorHandle()
	{
		m_SRVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12StructuredBufferSRVDescriptorHandle::Bind(StructuredBufferResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)resource;

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = resource->GetCount();
		srvDesc.Buffer.StructureByteStride = resource->GetStride();
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		context->GetDevice()->CreateShaderResourceView(dxResource->GetBuffer(), &srvDesc, m_SRVHandle.cpu);
	}

	DirectX12StructuredBufferUAVDescriptorHandle::DirectX12StructuredBufferUAVDescriptorHandle()
	{
		m_UAVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12StructuredBufferUAVDescriptorHandle::Bind(StructuredBufferResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12StructuredBufferResource* dxResource = (DirectX12StructuredBufferResource*)resource;

		D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
		uavDesc.Format = DXGI_FORMAT_UNKNOWN;
		uavDesc.ViewDimension = D3D12_UAV_DIMENSION_BUFFER;
		uavDesc.Buffer.FirstElement = 0;
		uavDesc.Buffer.NumElements = dxResource->GetCount();
		uavDesc.Buffer.StructureByteStride = dxResource->GetStride();
		uavDesc.Buffer.Flags = D3D12_BUFFER_UAV_FLAG_NONE;

		context->GetDevice()->CreateUnorderedAccessView(dxResource->GetBuffer(), nullptr, &uavDesc, m_UAVHandle.cpu);
	}

}

