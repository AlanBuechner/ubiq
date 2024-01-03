#include "pch.h"
#include "DirectX12ConstantBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

#include "Engine/Util/Performance.h"


namespace Engine
{

	DirectX12ConstantBufferResource::DirectX12ConstantBufferResource(uint32 size)
	{
		m_DefultState = ResourceState::ShaderResource;

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
			(D3D12_RESOURCE_STATES)GetState(m_DefultState), // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_Buffer)
		);
		CORE_ASSERT_HRESULT(hr, "Failed to create constant buffer resource");
	}

	DirectX12ConstantBufferResource::~DirectX12ConstantBufferResource()
	{
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void DirectX12ConstantBufferResource::SetData(const void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		context->GetDX12ResourceManager()->UploadBuffer(m_Buffer, data, m_Size, (D3D12_RESOURCE_STATES)GetState(m_DefultState));
	}

	uint32 DirectX12ConstantBufferResource::GetState(ResourceState state)
	{
		switch (state)
		{
		case ResourceState::ShaderResource:
			return D3D12_RESOURCE_STATE_VERTEX_AND_CONSTANT_BUFFER;
		case ResourceState::UnorderedResource:
			return D3D12_RESOURCE_STATE_UNORDERED_ACCESS;
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_COPY_SOURCE;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_COPY_DEST;
		}
	}

	DirectX12ConstantBufferCBVDescriptorHandle::DirectX12ConstantBufferCBVDescriptorHandle()
	{
		m_CBVHandle = DirectX12ResourceManager::s_SRVHeap->Allocate();
	}

	void DirectX12ConstantBufferCBVDescriptorHandle::Bind(ConstantBufferResource* resource)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		DirectX12ConstantBufferResource* dxResource = (DirectX12ConstantBufferResource*)resource;

		D3D12_CONSTANT_BUFFER_VIEW_DESC desc;
		desc.BufferLocation = dxResource->GetBuffer()->GetGPUVirtualAddress();
		desc.SizeInBytes = dxResource->GetSize();
		context->GetDevice()->CreateConstantBufferView(&desc, m_CBVHandle.cpu);
	}

}
