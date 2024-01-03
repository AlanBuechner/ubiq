#include "pch.h"
#include "DirectX12Buffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/Directx12Context.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	DirectX12VertexBufferResource::DirectX12VertexBufferResource(uint32 count, uint32 stride)
	{
		m_Count = count;
		m_Stride = stride;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

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
	}

	DirectX12VertexBufferResource::~DirectX12VertexBufferResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void DirectX12VertexBufferResource::SetData(const void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, m_Count * m_Stride, D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER);
	}

	uint32 DirectX12VertexBufferResource::GetState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		}
	}



	void DirectX12VertexBufferView::Bind(VertexBufferResource* resource)
	{
		DirectX12VertexBufferResource* dxResource = (DirectX12VertexBufferResource*)resource;
		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.StrideInBytes = dxResource->GetStride();
		m_View.SizeInBytes = dxResource->GetCount() * dxResource->GetStride();
	}








	// IndexBuffer ---------------------------------------------------------------------------------------

	DirectX12IndexBufferResource::DirectX12IndexBufferResource(uint32 count)
	{
		m_Count = count;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(count * sizeof(uint32));

		context->GetDevice()->CreateCommittedResource(
			&props, // a default heap
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // resource description for a buffer
			D3D12_RESOURCE_STATE_INDEX_BUFFER, // start in the copy destination state
			nullptr, // optimized clear value must be null for this type of resource
			IID_PPV_ARGS(&m_Buffer)
		);
	}

	DirectX12IndexBufferResource::~DirectX12IndexBufferResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void DirectX12IndexBufferResource::SetData(const void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, m_Count * sizeof(uint32), D3D12_RESOURCE_STATE_INDEX_BUFFER);
	}

	uint32 DirectX12IndexBufferResource::GetState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_INDEX_BUFFER;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		}
	}



	void DirectX12IndexBufferView::Bind(IndexBufferResource* resource)
	{
		DirectX12IndexBufferResource* dxResource = (DirectX12IndexBufferResource*)resource;
		m_View.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		m_View.SizeInBytes = dxResource->GetCount() * sizeof(uint32);
		m_View.Format = DXGI_FORMAT_R32_UINT;
	}

}
