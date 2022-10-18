#include "pch.h"
#include "DirectX12Texture.h"
#include "Directx12Context.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12CommandList.h"
#include "DirectX12ResourceManager.h"
#include "stb_image.h"

namespace Engine
{
	DirectX12Texture2D::DirectX12Texture2D(const fs::path& path, Ref<TextureAttribute> attrib)
		: m_Attribute(attrib)
	{
		LoadFromFile(path);
		CreateSampler();
	}

	DirectX12Texture2D::DirectX12Texture2D(const uint32 width, const uint32 height, Ref<TextureAttribute> attrib) :
		m_Width(width), m_Height(height), m_Attribute(attrib)
	{
		CreateImage(width, height);
		CreateSampler();
	}

	DirectX12Texture2D::~DirectX12Texture2D()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		m_Handle.Free();
		m_SamplerHandle.Free();
		context->GetDX12ResourceManager()->ScheduleResourceDeletion(m_Buffer);
	}

	void DirectX12Texture2D::SetData(void* data)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		// create upload buffer
		D3D12_RESOURCE_DESC rDesc;
		UINT uploadPitch = (m_Width * 4 + D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u) & ~(D3D12_TEXTURE_DATA_PITCH_ALIGNMENT - 1u);
		UINT uploadSize = m_Height * uploadPitch;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		rDesc.Alignment = 0;
		rDesc.Width = uploadSize;
		rDesc.Height = 1;
		rDesc.DepthOrArraySize = 1;
		rDesc.MipLevels = 1;
		rDesc.Format = DXGI_FORMAT_UNKNOWN;
		rDesc.SampleDesc.Count = 1;
		rDesc.SampleDesc.Quality = 0;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

		D3D12_HEAP_PROPERTIES props;
		props.Type = D3D12_HEAP_TYPE_UPLOAD;
		props.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
		props.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
		props.CreationNodeMask = 0;
		props.VisibleNodeMask = 0;

		wrl::ComPtr<ID3D12Resource> uploadBuffer;
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateCommittedResource(&props, D3D12_HEAP_FLAG_NONE, &rDesc,
			D3D12_RESOURCE_STATE_GENERIC_READ, 0, IID_PPV_ARGS(uploadBuffer.GetAddressOf())),
			"Failed To Create Upload Buffer");

		// map the date to the upload buffer
		void* mapped = nullptr;
		D3D12_RANGE range = { 0, uploadSize };
		CORE_ASSERT_HRESULT(uploadBuffer->Map(0, &range, &mapped), "Failed to map uplaod buffer");
		for (uint32 y = 0; y < m_Height; y++)
			memcpy((void*)((uintptr_t)mapped + y * uploadPitch), (stbi_uc*)data + y * m_Width * 4, m_Height * 4);
		//memcpy(mapped, data, m_Width * m_Height * 4);
		uploadBuffer->Unmap(0, &range);

		// copy the upload buffer to the resource
		D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
		srcLocation.pResource = uploadBuffer.Get();
		srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
		srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srcLocation.PlacedFootprint.Footprint.Width = m_Width;
		srcLocation.PlacedFootprint.Footprint.Height = m_Height;
		srcLocation.PlacedFootprint.Footprint.Depth = 1;
		srcLocation.PlacedFootprint.Footprint.RowPitch = uploadPitch;

		D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
		dstLocation.pResource = m_Buffer.Get();
		dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
		dstLocation.SubresourceIndex = 0;

		D3D12_RESOURCE_BARRIER barrier = {};
		barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
		barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
		barrier.Transition.pResource = m_Buffer.Get();
		barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
		barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
		barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PIXEL_SHADER_RESOURCE;

		Ref<CommandQueue> commandQueue = CommandQueue::Create(CommandQueue::Type::Direct);
		Ref<DirectX12CommandList> commandList = std::dynamic_pointer_cast<DirectX12CommandList>(CommandList::Create());
		commandList->StartRecording();
		commandList->GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
		commandList->GetCommandList()->ResourceBarrier(1, &barrier);
		commandList->Close();

		commandQueue->AddCommandList(commandList);
		commandQueue->Execute();
	}

	void DirectX12Texture2D::LoadFromFile(const fs::path& path)
	{
		m_Path = path;

		int width, height, channels;
		//stbi_set_flip_vertically_on_load(true);
		stbi_uc* data = stbi_load(path.string().c_str(), &width, &height, &channels, 4);
		CORE_ASSERT(data, "Failed to load image \"{0}\"", path);

		CreateImage(width, height);
		SetData(data);

		stbi_image_free(data);
	}

	void DirectX12Texture2D::CreateImage(uint32 width, uint32 height)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		if (m_Buffer)
		{
			m_Buffer->Release();
			m_Handle.Free();
		}

		m_Width = width;
		m_Height = height;

		D3D12_RESOURCE_DESC rDesc;
		rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
		rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
		rDesc.Flags = D3D12_RESOURCE_FLAG_NONE;
		rDesc.MipLevels = 1;
		rDesc.Width = width;
		rDesc.Height = height;
		rDesc.Alignment = 0;
		rDesc.DepthOrArraySize = 1;
		rDesc.SampleDesc = { 1, 0 };

		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
			D3D12_HEAP_FLAG_NONE,
			&rDesc,
			D3D12_RESOURCE_STATE_COPY_DEST,
			nullptr, 
			IID_PPV_ARGS(m_Buffer.GetAddressOf())
		);

		m_Handle = DirectX12ResourceManager::s_SRVHeap->Allocate();
		D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc = {};
		srvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
		srvDesc.Texture2D.MipLevels = rDesc.MipLevels;
		srvDesc.Texture2D.MostDetailedMip = 0;
		srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;

		context->GetDevice()->CreateShaderResourceView(m_Buffer.Get(), &srvDesc, m_Handle.cpu);
	}

	void DirectX12Texture2D::CreateSampler()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		m_SamplerHandle = DirectX12ResourceManager::s_SamplerHeap->Allocate();

		D3D12_SAMPLER_DESC desc{};
		desc.AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
		desc.Filter = D3D12_FILTER_ANISOTROPIC;

		context->GetDevice()->CreateSampler(&desc, m_SamplerHandle.cpu);
	}

}
