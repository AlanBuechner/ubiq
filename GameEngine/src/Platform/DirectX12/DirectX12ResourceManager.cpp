#include "pch.h"
#include "DirectX12ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include <algorithm>


Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SRVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_RTVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_DSVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SamplerHeap;

namespace Engine
{

	// deletion pool
	void DirectX12ResourceDeletionPool::Clear()
	{
		m_Pool.clear();

		for (DirectX12DescriptorHandle h : m_HandlePool)
			h.Free();
		m_HandlePool.clear();
	}

	void DirectX12ResourceDeletionPool::AddResource(wrl::ComPtr<ID3D12Resource> resource)
	{
		m_Pool.push_back(resource);
	}

	void DirectX12ResourceDeletionPool::AddHandle(DirectX12DescriptorHandle handle)
	{
		m_HandlePool.push_back(handle);
	}

	// upload page
	DirectX12UploadPage::DirectX12UploadPage(uint32 size) :
		m_Size(size), m_UsedMemory(0)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(size), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&m_UploadBuffer)
		);

		CD3DX12_RANGE readRange(0, 0);
		m_UploadBuffer->Map(0, &readRange, &m_BasePointer);
		m_MemWrightPointer = m_BasePointer;
	}


	DirectX12UploadPage::~DirectX12UploadPage()
	{
		//D3D12_RANGE range = { 0, 0 };
		//m_UploadBuffer->Unmap(0, &range);
	}

	void* DirectX12UploadPage::Map(const void* data, uint32 size)
	{
		// if the amount of memory requested to upload is greater than the amount of available space return nullptr 
		if ((m_Size - m_UsedMemory) < size)
			return nullptr;

		void* loc = m_MemWrightPointer;
		memcpy(loc, data, size);
		(uint64&)m_MemWrightPointer += size;
		m_UsedMemory += size;

		return loc;
	}

	uint64 DirectX12UploadPage::GetOffset(const void* data)
	{
		return (uint64)data - (uint64)m_BasePointer;
	}

	void DirectX12UploadPage::Clear()
	{
		m_UsedMemory = 0;
		m_MemWrightPointer = m_BasePointer;
	}

	// resource manager
	DirectX12ResourceManager::DirectX12ResourceManager()
	{
		s_SRVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
		s_RTVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
		s_DSVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
		s_SamplerHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);

		m_DeletionPool = CreateRef<DirectX12ResourceDeletionPool>();
	}

	DirectX12ResourceManager::~DirectX12ResourceManager()
	{
		s_SRVHeap.reset();
		s_RTVHeap.reset();
		s_DSVHeap.reset();
		s_SamplerHeap.reset();
	}

	wrl::ComPtr<ID3D12Resource> DirectX12ResourceManager::CreateUploadTexture2D(DXGI_FORMAT format, uint32 width, uint32 height)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		wrl::ComPtr<ID3D12Resource> constantBufferUploadHeap;
		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Tex2D(format, width, height), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeap)
		);

		return constantBufferUploadHeap;
	}

	wrl::ComPtr<ID3D12Resource> DirectX12ResourceManager::CreateUploadBuffer(uint32 size)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		wrl::ComPtr<ID3D12Resource> constantBufferUploadHeap;
		context->GetDevice()->CreateCommittedResource(
			&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&CD3DX12_RESOURCE_DESC::Buffer(size), // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeap)
		);

		return constantBufferUploadHeap;
	}

	void DirectX12ResourceManager::UploadBuffer(wrl::ComPtr<ID3D12Resource> dest, const void* data, uint32 size, D3D12_RESOURCE_STATES state)
	{
		UploadBufferRegion(dest, 0, data, size, state);
	}

	void DirectX12ResourceManager::UploadBufferRegion(wrl::ComPtr<ID3D12Resource> dest, uint64 offset, const void* data, uint32 size, D3D12_RESOURCE_STATES state)
	{
		std::lock_guard g(m_UploadMutex);

		DirectX12UploadPage* page;
		void* loc = nullptr;
		for (uint32 i = 0; i < m_UploadPages.size(); i++)
		{
			loc = m_UploadPages[i].Map(data, size);
			if (loc != nullptr)
			{
				page = &m_UploadPages[i];
				break;
			}
		}

		if (loc == nullptr)
		{
			m_UploadPages.push_back(DirectX12UploadPage(size * 2)); // create new page double the size needed
			page = &m_UploadPages.back();
			loc = page->Map(data, size);
			CORE_ASSERT(loc != nullptr, "Failed to upload data");
		}

		UploadBufferData uploadData;
		uploadData.size = size;
		uploadData.destOffset = offset;
		uploadData.srcOffset = page->GetOffset(loc);
		uploadData.destResource = dest;
		uploadData.uploadResource = page->GetResource();
		uploadData.state = state;

		m_BufferUploadQueue.push(uploadData);
	}

	void DirectX12ResourceManager::CopyBuffer(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 size, D3D12_RESOURCE_STATES state)
	{
		UploadBufferData uploadData;
		uploadData.size = size;
		uploadData.destOffset = 0;
		uploadData.destResource = dest;
		uploadData.srcOffset = 0;
		uploadData.uploadResource = src;
		uploadData.state = state;

		m_BufferUploadQueue.push(uploadData);
	}

	void DirectX12ResourceManager::UploadTexture(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 width, uint32 height, uint32 pitch, bool genMipChain, D3D12_RESOURCE_STATES state)
	{
		UploadTextureData uploadData;
		uploadData.destResource = dest;
		uploadData.uploadResource = src;
		uploadData.width = width;
		uploadData.height = height;
		uploadData.pitch = pitch;
		uploadData.state = state;

		uploadData.genMipChain = genMipChain;
		if (std::max(width, height) == 1)
			uploadData.genMipChain = false;

		m_TextureUploadQueue.push(uploadData);
	}

	Ref<ResourceDeletionPool> DirectX12ResourceManager::CreateNewDeletionPool()
	{
		Ref<ResourceDeletionPool> pool = m_DeletionPool;
		m_DeletionPool = CreateRef<DirectX12ResourceDeletionPool>();
		return pool;
	}

	void DirectX12ResourceManager::RecordCommands(Ref<CommandList> commandList)
	{
		std::lock_guard g(m_UploadMutex);
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		struct BufferCopyInfo
		{
			ID3D12Resource* dest;
			ID3D12Resource* src;
			uint64 destOffset;
			uint64 srcOffset;
			uint64 size;
		};

		struct TextureCopyInfo
		{
			ID3D12Resource* dest;
			ID3D12Resource* src;
			uint32 width;
			uint32 height;
			uint32 pitch;
		};

		struct TextureMipInfo
		{
			wrl::ComPtr<ID3D12Resource> texture;
			wrl::ComPtr<ID3D12Resource> mipTexture;
			uint32 width;
			uint32 height;
			uint32 numMips;
			D3D12_RESOURCE_STATES state;
		};

		CREATE_PROFILE_FUNCTIONI();
		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.size();
		uint32 numTextureCopys = (uint32)m_TextureUploadQueue.size();
		std::vector<D3D12_RESOURCE_BARRIER> startb;
		std::vector<D3D12_RESOURCE_BARRIER> endb;
		std::vector<BufferCopyInfo> bufferCopys;
		std::vector<TextureCopyInfo> textureCopys;
		std::vector<TextureMipInfo> mipTextures;

		startb.reserve(numBufferCopys);
		endb.reserve(numBufferCopys);
		bufferCopys.reserve(numBufferCopys);
		textureCopys.reserve(numTextureCopys);

		while (!m_BufferUploadQueue.empty())
		{
			UploadBufferData& data = m_BufferUploadQueue.front();

			startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST));
			bufferCopys.push_back({ data.destResource.Get(), data.uploadResource.Get(), data.destOffset, data.srcOffset, data.size });
			endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, data.state));

			m_BufferUploadQueue.pop();
		}

		while (!m_TextureUploadQueue.empty())
		{
			UploadTextureData& data = m_TextureUploadQueue.front();

			if (data.genMipChain)
			{
				// create temp texture to generate the mip levels
				wrl::ComPtr<ID3D12Resource> mipTexture;

				D3D12_RESOURCE_DESC rDesc;
				rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				rDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				rDesc.MipLevels = (uint32)std::floor(std::log2(std::max(data.width, data.height))) + 1;
				rDesc.Width = data.width;
				rDesc.Height = data.height;
				rDesc.Alignment = 0;
				rDesc.DepthOrArraySize = 1;
				rDesc.SampleDesc = { 1, 0 };

				context->GetDevice()->CreateCommittedResource(
					&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT),
					D3D12_HEAP_FLAG_NONE,
					&rDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(mipTexture.GetAddressOf())
				);
				mipTexture->SetName(L"MipTexture");
				mipTextures.push_back({ data.destResource, mipTexture, data.width, data.height, rDesc.MipLevels, data.state });

				startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST)); // transition final texture to copy destination
				textureCopys.push_back({ mipTexture.Get(), data.uploadResource.Get(), data.width, data.height, data.pitch });
				endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			}
			else
			{
				startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST));
				textureCopys.push_back({ data.destResource.Get(), data.uploadResource.Get(), data.width, data.height, data.pitch });
				endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, data.state));
			}

			m_TextureUploadQueue.pop();
		}

		// get the command list
		Ref<DirectX12CommandList> dxCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(commandList);

		dxCommandList->StartRecording(); // start recording 
		dxCommandList->GetCommandList()->ResourceBarrier((uint32)startb.size(), startb.data()); // transition resources to copy

		// copy buffers
		for (uint32 i = 0; i < numBufferCopys; i++)
			dxCommandList->GetCommandList()->CopyBufferRegion(bufferCopys[i].dest, bufferCopys[i].destOffset, bufferCopys[i].src, bufferCopys[i].srcOffset, bufferCopys[i].size);

		// copy textures
		for (uint32 i = 0; i < numTextureCopys; i++)
		{
			TextureCopyInfo& info = textureCopys[i];

			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = info.src;
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLocation.PlacedFootprint.Footprint.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
			srcLocation.PlacedFootprint.Footprint.Width = info.width;
			srcLocation.PlacedFootprint.Footprint.Height = info.height;
			srcLocation.PlacedFootprint.Footprint.Depth = 1;
			srcLocation.PlacedFootprint.Footprint.RowPitch = info.pitch;

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = info.dest;
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLocation.SubresourceIndex = 0;
			dxCommandList->GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
		}

		dxCommandList->GetCommandList()->ResourceBarrier((uint32)endb.size(), endb.data()); // transition resources back to original state


		// generate mip maps
		std::vector<D3D12_RESOURCE_BARRIER> mipBarrier;
		std::vector<D3D12_RESOURCE_BARRIER> textureBarrier;
		std::vector<TextureCopyInfo> mipCopys;

		for (auto& mipTexture : mipTextures)
		{
			for (uint32 topMip = 0; topMip < mipTexture.numMips - 1; topMip++)
			{
				uint32_t srcWidth = std::max<uint32>(mipTexture.width >> (topMip), 1);
				uint32_t srcHeight = std::max<uint32>(mipTexture.height >> (topMip), 1);

				uint32_t dstWidth = std::max<uint32>(mipTexture.width >> (topMip + 1), 1);
				uint32_t dstHeight = std::max<uint32>(mipTexture.height >> (topMip + 1), 1);

				DirectX12DescriptorHandle lastMip;
				DirectX12DescriptorHandle currMip;

				// create SRV for last mip level
				lastMip = s_SRVHeap->Allocate();
				D3D12_UNORDERED_ACCESS_VIEW_DESC lastUavDesc = {};
				lastUavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				lastUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				lastUavDesc.Texture2D.MipSlice = topMip;
				context->GetDevice()->CreateUnorderedAccessView(mipTexture.mipTexture.Get(), nullptr, &lastUavDesc, lastMip.cpu);

				// create UAV for current mip level
				currMip = s_SRVHeap->Allocate();
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = topMip + 1;
				context->GetDevice()->CreateUnorderedAccessView(mipTexture.mipTexture.Get(), nullptr, &uavDesc, currMip.cpu);

				// blit the last mip level onto the current mip level
				Ref<ComputeShader> blit = Renderer::GetBlitShader();
				dxCommandList->SetComputeShader(blit);
				dxCommandList->GetCommandList()->SetComputeRoot32BitConstant(blit->GetUniformLocation("RC_CBX"), *(uint32*)(void*)&srcWidth, 0);
				dxCommandList->GetCommandList()->SetComputeRoot32BitConstant(blit->GetUniformLocation("RC_CBY"), *(uint32*)(void*)&srcHeight, 0);
				dxCommandList->GetCommandList()->SetComputeRootDescriptorTable(blit->GetUniformLocation("SrcTexture"), lastMip.gpu);
				dxCommandList->GetCommandList()->SetComputeRootDescriptorTable(blit->GetUniformLocation("DstTexture"), currMip.gpu);
				dxCommandList->GetCommandList()->Dispatch(std::max(dstWidth / 8, 1u) + 1, std::max(dstHeight / 8, 1u) + 1, 1);

				dxCommandList->GetCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::UAV(mipTexture.mipTexture.Get()));

				// delete handles
				ScheduleHandelDeletion(lastMip);
				ScheduleHandelDeletion(currMip);
			}

			// record barrier for texture and mip
			mipBarrier.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.mipTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
			textureBarrier.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, mipTexture.state));

			// copy resource to final texture
			mipCopys.push_back({ mipTexture.texture.Get(), mipTexture.mipTexture.Get(), 0,0,0 });

			// schedule freeing of mip texture
			ScheduleResourceDeletion(mipTexture.mipTexture);
		}

		dxCommandList->GetCommandList()->ResourceBarrier((uint32)mipBarrier.size(), mipBarrier.data()); // transition mips to copy src
		for (uint32 i = 0; i < mipCopys.size(); i++)
			dxCommandList->GetCommandList()->CopyResource(mipCopys[i].dest, mipCopys[i].src);
		dxCommandList->GetCommandList()->ResourceBarrier((uint32)textureBarrier.size(), textureBarrier.data()); // transition resources back to original state

		dxCommandList->Close();

		// clear upload pages
		for (uint32 i = 0; i < m_UploadPages.size(); i++)
			m_UploadPages[i].Clear();
	}
}
