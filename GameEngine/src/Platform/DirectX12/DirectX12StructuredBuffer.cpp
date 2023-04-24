#include "pch.h"
#include "DirectX12StructuredBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12Context.h"

namespace Engine
{

	DirectX12StructuredBuffer::DirectX12StructuredBuffer(uint32 stride, uint32 count) :
		m_Stride(stride), m_Count(count)
	{
		Resize(count);

	}

	DirectX12StructuredBuffer::~DirectX12StructuredBuffer()
	{
		DirectX12ResourceManager::s_SRVHeap->Free(m_SRVhandle);
	}

	void DirectX12StructuredBuffer::Resize(uint32 count)
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

		m_SRVhandle = DirectX12ResourceManager::s_SRVHeap->Allocate();

		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
		srvDesc.Format = DXGI_FORMAT_UNKNOWN;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
		srvDesc.Buffer.FirstElement = 0;
		srvDesc.Buffer.NumElements = count;
		srvDesc.Buffer.StructureByteStride = m_Stride;
		srvDesc.Buffer.Flags = D3D12_BUFFER_SRV_FLAG_NONE;

		context->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_SRVhandle.cpu);
	}

	void DirectX12StructuredBuffer::SetData(void* data, uint32 count /*= 1*/, uint32 start /*= 0*/)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBufferRegion(m_Buffer, m_Stride * start, data, m_Stride * count, D3D12_RESOURCE_STATE_NON_PIXEL_SHADER_RESOURCE | D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE);
	}

	uint32 DirectX12StructuredBuffer::GetDescriptorLocation() const
	{
		return m_SRVhandle.GetIndex();
	}

}

