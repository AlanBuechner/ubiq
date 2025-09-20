#include "DirectX12UploadBuffer.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"

namespace Engine
{
	DirectX12UploadBufferResource::DirectX12UploadBufferResource(uint32 size)
	{
		m_DefultState = ResourceState::CopySource;
		m_Size = size;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		{
			CREATE_PROFILE_SCOPEI("Create Committed");
			context->GetDevice()->CreateCommittedResource(
				&props, // this heap will be used to upload the constant buffer data
				D3D12_HEAP_FLAG_NONE, // no flags
				&resDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
				D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
				nullptr, // we do not have use an optimized clear value for constant buffers
				IID_PPV_ARGS(&m_Buffer)
			);
		}

	}

	DirectX12UploadBufferResource::~DirectX12UploadBufferResource()
	{
		CREATE_PROFILE_FUNCTIONI();
		if (m_MapedBasePointer != nullptr)
			UnMap();
		m_Buffer->Release();
		m_Buffer = nullptr;
		m_MapedBasePointer = nullptr;
	}

	void* DirectX12UploadBufferResource::Map()
	{
		CREATE_PROFILE_FUNCTIONI();
		CD3DX12_RANGE readRange(0, 0);
		m_Buffer->Map(0, &readRange, &m_MapedBasePointer);
		return m_MapedBasePointer;
	}

	void DirectX12UploadBufferResource::UnMap()
	{
		CREATE_PROFILE_FUNCTIONI();
		D3D12_RANGE range = { 0, 0 };
		m_Buffer->Unmap(0, &range);
		m_MapedBasePointer = nullptr;
	}

	uint32 DirectX12UploadBufferResource::GetGPUState(ResourceState state) const
	{
		switch (state)
		{
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		default:
			return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	void DirectX12UploadBufferResource::AllocateTransient(class TransientResourceHeap* heap, uint32 offset)
	{
		CORE_ASSERT(false, "Cant make transient constant buffer", "");
	}






	DirectX12UploadTextureResource::DirectX12UploadTextureResource(uint32 width, uint32 height, uint32 mips, TextureFormat format)
	{
		m_DefultState = ResourceState::CopySource;
		m_Width = width;
		m_Height = height;
		m_Mips = mips;
		m_Format = format;
		m_Pitch = (m_Width * GetStride() + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		D3D12_RESOURCE_DESC rDesc;
		UINT stride = GetStride();
		UINT size = GetUploadSize();

		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Alignment = 0;
		rDesc.Width = size;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		CORE_ASSERT_HRESULT(
			context->GetDevice()->CreateCommittedResource(
				&props,
				D3D12_HEAP_FLAG_NONE,
				&resDesc,
				D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(&m_Buffer)
			), "Failed To Create Upload Buffer"
		);

	}

	DirectX12UploadTextureResource::~DirectX12UploadTextureResource()
	{
		if (m_MapedBasePointer != nullptr)
			UnMap();
		m_Buffer->Release();
		m_Buffer = nullptr;
	}

	void* DirectX12UploadTextureResource::Map()
	{
		D3D12_RANGE range = { 0, GetUploadSize() };
		m_Buffer->Map(0, &range, &m_MapedBasePointer);
		return m_MapedBasePointer;
	}

	void DirectX12UploadTextureResource::UnMap()
	{
		D3D12_RANGE range = { 0, GetUploadSize() };
		m_Buffer->Unmap(0, &range);
		m_MapedBasePointer = nullptr;
	}

	uint32 DirectX12UploadTextureResource::GetGPUState(ResourceState state) const
	{
		switch (state)
		{
		case ResourceState::CopySource:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		case ResourceState::CopyDestination:
			return D3D12_RESOURCE_STATE_GENERIC_READ;
		default:
			return D3D12_RESOURCE_STATE_COMMON;
		}
	}

	void DirectX12UploadTextureResource::AllocateTransient(class TransientResourceHeap* heap, uint32 offset)
	{
		CORE_ASSERT(false, "Cant make transient constant buffer", "");
	}

}

