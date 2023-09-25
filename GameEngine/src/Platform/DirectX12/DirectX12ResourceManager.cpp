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

	// upload page
	DirectX12UploadPage::DirectX12UploadPage(uint32 size) :
		m_Size(size), m_UsedMemory(0)
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		context->GetDevice()->CreateCommittedResource(
			&props, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
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
	DirectX12ResourceManager::DirectX12ResourceManager() :
		ResourceManager()
	{
		s_SRVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
		s_RTVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
		s_DSVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
		s_SamplerHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);

		//m_CommandQueue = std::dynamic_pointer_cast<DirectX12CommandQueue>(CommandQueue::Create(CommandQueue::Type::Direct));
		m_BufferCopyCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(CommandList::Create(CommandList::Direct));
		m_TextureCopyCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(CommandList::Create(CommandList::Direct));

		m_UploadDescriptors = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
	}

	DirectX12ResourceManager::~DirectX12ResourceManager()
	{
		delete m_DeletionPool; // needs to be destroyed before the heaps

		s_SRVHeap.reset();
		s_RTVHeap.reset();
		s_DSVHeap.reset();
		s_SamplerHeap.reset();
	}

	wrl::ComPtr<ID3D12Resource> DirectX12ResourceManager::CreateUploadTexture2D(DXGI_FORMAT format, uint32 width, uint32 height)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Tex2D(format, width, height);

		wrl::ComPtr<ID3D12Resource> constantBufferUploadHeap;
		context->GetDevice()->CreateCommittedResource(
			&props, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
			D3D12_RESOURCE_STATE_GENERIC_READ, // will be data that is read from so we keep it in the generic read state
			nullptr, // we do not have use an optimized clear value for constant buffers
			IID_PPV_ARGS(&constantBufferUploadHeap)
		);

		return constantBufferUploadHeap;
	}

	ID3D12Resource* DirectX12ResourceManager::CreateUploadBuffer(uint32 size)
	{
		CREATE_PROFILE_FUNCTIONI();
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD);
		CD3DX12_RESOURCE_DESC resDesc = CD3DX12_RESOURCE_DESC::Buffer(size);

		ID3D12Resource* constantBufferUploadHeap;
		context->GetDevice()->CreateCommittedResource(
			&props, // this heap will be used to upload the constant buffer data
			D3D12_HEAP_FLAG_NONE, // no flags
			&resDesc, // size of the resource heap. Must be a multiple of 64KB for single-textures and constant buffers
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

		if (dest.Get() == nullptr)
		{
			CORE_ERROR("atempting to upload data with null resource");
			__debugbreak();
			return;
		}

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
		if (dest.Get() == nullptr || src.Get() == nullptr)
		{
			CORE_ERROR("atempting to upload data with null resource");
			__debugbreak();
			return;
		}

		std::lock_guard g(m_UploadMutex);
		UploadBufferData uploadData;
		uploadData.size = size;
		uploadData.destOffset = 0;
		uploadData.destResource = dest;
		uploadData.srcOffset = 0;
		uploadData.uploadResource = src;
		uploadData.state = state;

		m_BufferUploadQueue.push(uploadData);
	}

	void DirectX12ResourceManager::UploadTexture(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 width, uint32 height, uint32 pitch, uint32 numMips, D3D12_RESOURCE_STATES state, DXGI_FORMAT format)
	{

		if (dest.Get() == nullptr || src.Get() == nullptr)
		{
			CORE_ERROR("atempting to upload data with null resource");
			__debugbreak();
			return;
		}

		std::lock_guard g(m_UploadMutex);
		UploadTextureData uploadData;
		uploadData.destResource = dest;
		uploadData.uploadResource = src;
		uploadData.width = width;
		uploadData.height = height;
		uploadData.pitch = pitch;
		uploadData.numMips = numMips;
		uploadData.state = state;
		uploadData.format = format;

		m_TextureUploadQueue.push(uploadData);
	}

	void DirectX12ResourceManager::UploadData()
	{
		CREATE_PROFILE_FUNCTIONI();

		InstrumentationTimer timer = CREATE_PROFILEI();
		std::lock_guard g(m_UploadMutex);
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		timer.Start("Record commands");
		RecordBufferCommands(context);
		RecordTextureCommands(context);
		timer.End();

		//m_CommandQueue->Submit(m_BufferCopyCommandList);
		//m_CommandQueue->Submit(m_TextureCopyCommandList);
		//m_CommandQueue->Build();
		//m_CommandQueue->Execute();
		//m_CommandQueue->Await();
		
	}

	void DirectX12ResourceManager::Clean()
	{
		// clear upload pages
		for (uint32 i = 0; i < m_UploadPages.size(); i++)
			m_UploadPages[i].Clear();
	}

	void DirectX12ResourceManager::RecordBufferCommands(Ref<DirectX12Context> context)
	{
		CREATE_PROFILE_FUNCTIONI();

		// structs
		struct BufferCopyInfo
		{
			ID3D12Resource* dest;
			ID3D12Resource* src;
			uint64 destOffset;
			uint64 srcOffset;
			uint64 size;
		};


		// data
		std::vector<D3D12_RESOURCE_BARRIER> startb;
		std::vector<D3D12_RESOURCE_BARRIER> endb;
		std::vector<BufferCopyInfo> bufferCopys;

		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.size();
		startb.reserve(numBufferCopys);
		endb.reserve(numBufferCopys);
		bufferCopys.reserve(numBufferCopys);

		std::unordered_set<ID3D12Resource*> m_SeenUploads;

		// collect data
		while (!m_BufferUploadQueue.empty())
		{
			UploadBufferData& data = m_BufferUploadQueue.front();

			if (m_SeenUploads.find(data.destResource.Get()) == m_SeenUploads.end())
			{
				startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST));
				endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, data.state));
				m_SeenUploads.insert(data.destResource.Get());
			}
			bufferCopys.push_back({ data.destResource.Get(), data.uploadResource.Get(), data.destOffset, data.srcOffset, data.size });

			m_BufferUploadQueue.pop();
		}

		// record commands

		m_BufferCopyCommandList->StartRecording(); // start recording 
		m_BufferCopyCommandList->GetCommandList()->ResourceBarrier((uint32)startb.size(), startb.data()); // transition resources to copy dest

		for (uint32 i = 0; i < numBufferCopys; i++)
		{
			m_BufferCopyCommandList->GetCommandList()->CopyBufferRegion(
				bufferCopys[i].dest,
				bufferCopys[i].destOffset,
				bufferCopys[i].src,
				bufferCopys[i].srcOffset,
				bufferCopys[i].size
			);
		}

		m_BufferCopyCommandList->GetCommandList()->ResourceBarrier((uint32)endb.size(), endb.data()); // transition resources back to original state

		m_BufferCopyCommandList->Close();

	}

	void DirectX12ResourceManager::RecordTextureCommands(Ref<DirectX12Context> context)
	{
		m_UploadDescriptors->Clear();
		m_TempUplaodResourecs.clear();
		CREATE_PROFILE_FUNCTIONI();
		// structs
		struct TextureCopyInfo
		{
			ID3D12Resource* dest;
			ID3D12Resource* src;
			uint32 width;
			uint32 height;
			uint32 pitch;
			DXGI_FORMAT format;
		};

		struct TextureMipInfo
		{
			wrl::ComPtr<ID3D12Resource> texture;
			wrl::ComPtr<ID3D12Resource> mipTexture;
			uint32 width;
			uint32 height;
			uint32 numMips;
			D3D12_RESOURCE_STATES state;
			DXGI_FORMAT format;
		};

		// data
		uint32 numTextureCopys = (uint32)m_TextureUploadQueue.size();

		std::vector<D3D12_RESOURCE_BARRIER> startb;
		std::vector<D3D12_RESOURCE_BARRIER> endb;
		std::vector<TextureCopyInfo> textureCopys;
		std::vector<TextureMipInfo> mipTextures;

		textureCopys.reserve(numTextureCopys);

		// collect data
		while (!m_TextureUploadQueue.empty())
		{
			UploadTextureData& data = m_TextureUploadQueue.front();

			if (data.numMips > 1)
			{
				// create temp texture to generate the mip levels
				wrl::ComPtr<ID3D12Resource> mipTexture;

				CD3DX12_HEAP_PROPERTIES props = CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT);

				D3D12_RESOURCE_DESC rDesc;
				rDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
				rDesc.Format = data.format;
				rDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
				rDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_UNORDERED_ACCESS;
				rDesc.MipLevels = data.numMips;
				rDesc.Width = data.width;
				rDesc.Height = data.height;
				rDesc.Alignment = 0;
				rDesc.DepthOrArraySize = 1;
				rDesc.SampleDesc = { 1, 0 };

				context->GetDevice()->CreateCommittedResource(
					&props,
					D3D12_HEAP_FLAG_NONE,
					&rDesc,
					D3D12_RESOURCE_STATE_COPY_DEST,
					nullptr,
					IID_PPV_ARGS(mipTexture.GetAddressOf())
				);
				mipTexture->SetName(L"MipTexture");
				mipTextures.push_back({ data.destResource, mipTexture, data.width, data.height, rDesc.MipLevels, data.state, data.format });
				m_TempUplaodResourecs.push_back(mipTexture);

				startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST)); // transition final texture to copy destination
				textureCopys.push_back({ mipTexture.Get(), data.uploadResource.Get(), data.width, data.height, data.pitch, data.format });
				endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, D3D12_RESOURCE_STATE_UNORDERED_ACCESS));
			}
			else
			{
				startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST));
				textureCopys.push_back({ data.destResource.Get(), data.uploadResource.Get(), data.width, data.height, data.pitch, data.format});
				endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, data.state));
			}

			m_TextureUploadQueue.pop();
		}


		// start recording 
		m_TextureCopyCommandList->StartRecording(); // start recording 
		ID3D12DescriptorHeap* heaps[]{ m_UploadDescriptors->GetHeap().Get()};
		m_TextureCopyCommandList->GetCommandList()->SetDescriptorHeaps(1, heaps);
		m_TextureCopyCommandList->GetCommandList()->ResourceBarrier((uint32)startb.size(), startb.data()); // transition resources to copy

		// copy textures
		for (uint32 i = 0; i < textureCopys.size(); i++)
		{
			TextureCopyInfo& info = textureCopys[i];

			D3D12_TEXTURE_COPY_LOCATION srcLocation = {};
			srcLocation.pResource = info.src;
			srcLocation.Type = D3D12_TEXTURE_COPY_TYPE_PLACED_FOOTPRINT;
			srcLocation.PlacedFootprint.Offset = 0;
			srcLocation.PlacedFootprint.Footprint.Format = info.format;
			srcLocation.PlacedFootprint.Footprint.Width = info.width;
			srcLocation.PlacedFootprint.Footprint.Height = info.height;
			srcLocation.PlacedFootprint.Footprint.Depth = 1;
			srcLocation.PlacedFootprint.Footprint.RowPitch = info.pitch;

			D3D12_TEXTURE_COPY_LOCATION dstLocation = {};
			dstLocation.pResource = info.dest;
			dstLocation.Type = D3D12_TEXTURE_COPY_TYPE_SUBRESOURCE_INDEX;
			dstLocation.SubresourceIndex = 0;
			m_TextureCopyCommandList->GetCommandList()->CopyTextureRegion(&dstLocation, 0, 0, 0, &srcLocation, NULL);
		}

		m_TextureCopyCommandList->GetCommandList()->ResourceBarrier((uint32)endb.size(), endb.data()); // transition resources back to original state


		// generate mip maps
		std::vector<D3D12_RESOURCE_BARRIER> mipBarrier;
		std::vector<D3D12_RESOURCE_BARRIER> textureBarrier;
		std::vector<TextureCopyInfo> mipCopys;

		for (auto& mipTexture : mipTextures)
		{
			for (uint32 topMip = 0; topMip < mipTexture.numMips - 1; topMip++)
			{
				uint32 dstWidth = std::max<uint32>(mipTexture.width >> (topMip + 1), 1);
				uint32 dstHeight = std::max<uint32>(mipTexture.height >> (topMip + 1), 1);

				DirectX12DescriptorHandle lastMip;
				DirectX12DescriptorHandle currMip;

				// create SRV for last mip level
				lastMip = m_UploadDescriptors->Allocate();
				D3D12_UNORDERED_ACCESS_VIEW_DESC lastUavDesc = {};
				lastUavDesc.Format = mipTexture.format;
				lastUavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				lastUavDesc.Texture2D.MipSlice = topMip;
				context->GetDevice()->CreateUnorderedAccessView(mipTexture.mipTexture.Get(), nullptr, &lastUavDesc, lastMip.cpu);

				// create UAV for current mip level
				currMip = m_UploadDescriptors->Allocate();
				D3D12_UNORDERED_ACCESS_VIEW_DESC uavDesc = {};
				uavDesc.Format = mipTexture.format;
				uavDesc.ViewDimension = D3D12_UAV_DIMENSION_TEXTURE2D;
				uavDesc.Texture2D.MipSlice = topMip + 1;
				context->GetDevice()->CreateUnorderedAccessView(mipTexture.mipTexture.Get(), nullptr, &uavDesc, currMip.cpu);

				// blit the last mip level onto the current mip level
				Ref<Shader> blit = Renderer::GetBlitShader();
				Ref<ShaderPass> blitPass = blit->GetPass("Blit");
				m_TextureCopyCommandList->SetShader(blitPass);
				m_TextureCopyCommandList->GetCommandList()->SetComputeRootDescriptorTable(blitPass->GetUniformLocation("SrcTexture"), lastMip.gpu);
				m_TextureCopyCommandList->GetCommandList()->SetComputeRootDescriptorTable(blitPass->GetUniformLocation("DstTexture"), currMip.gpu);
				m_TextureCopyCommandList->Dispatch(std::max(dstWidth / 8, 1u) + 1, std::max(dstHeight / 8, 1u) + 1, 1);

				CD3DX12_RESOURCE_BARRIER barrier = CD3DX12_RESOURCE_BARRIER::UAV(mipTexture.mipTexture.Get());
				m_TextureCopyCommandList->GetCommandList()->ResourceBarrier(1, &barrier);
			}


			// record barrier for texture and mip
			mipBarrier.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.mipTexture.Get(), D3D12_RESOURCE_STATE_UNORDERED_ACCESS, D3D12_RESOURCE_STATE_COPY_SOURCE));
			textureBarrier.push_back(CD3DX12_RESOURCE_BARRIER::Transition(mipTexture.texture.Get(), D3D12_RESOURCE_STATE_COPY_DEST, mipTexture.state));

			// copy resource to final texture
			mipCopys.push_back({ mipTexture.texture.Get(), mipTexture.mipTexture.Get(), 0,0,0 });
		}

		m_TextureCopyCommandList->GetCommandList()->ResourceBarrier((uint32)mipBarrier.size(), mipBarrier.data()); // transition mips to copy src
		for (uint32 i = 0; i < mipCopys.size(); i++)
			m_TextureCopyCommandList->GetCommandList()->CopyResource(mipCopys[i].dest, mipCopys[i].src);
		m_TextureCopyCommandList->GetCommandList()->ResourceBarrier((uint32)textureBarrier.size(), textureBarrier.data()); // transition resources back to original state

		m_TextureCopyCommandList->Close();
	}

}
