#include "pch.h"
#include "DirectX12ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"


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
	}

	void DirectX12ResourceDeletionPool::AddResource(wrl::ComPtr<ID3D12Resource> resource)
	{
		m_Pool.push_back(resource);
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

	void DirectX12ResourceManager::UploadTexture(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 width, uint32 height, uint32 pitch, D3D12_RESOURCE_STATES state)
	{
		UploadTextureData uploadData;
		uploadData.destResource = dest;
		uploadData.uploadResource = src;
		uploadData.width = width;
		uploadData.height = height;
		uploadData.pitch = pitch;
		uploadData.state = state;

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
			uint64 width;
			uint64 height;
			uint64 pitch;
		};

		CREATE_PROFILE_FUNCTIONI();
		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.size();
		uint32 numTextureCopys = (uint32)m_TextureUploadQueue.size();
		std::vector<D3D12_RESOURCE_BARRIER> startb;
		std::vector<D3D12_RESOURCE_BARRIER> endb;
		std::vector<BufferCopyInfo> bufferCopys;
		std::vector<TextureCopyInfo> textureCopys;

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

			startb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), data.state, D3D12_RESOURCE_STATE_COPY_DEST));
			textureCopys.push_back({ data.destResource.Get(), data.uploadResource.Get(), data.width, data.height, data.pitch });
			endb.push_back(CD3DX12_RESOURCE_BARRIER::Transition(data.destResource.Get(), D3D12_RESOURCE_STATE_COPY_DEST, data.state));

			m_TextureUploadQueue.pop();
		}

		// get the command list
		Ref<DirectX12CommandList> dxCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(commandList);

		dxCommandList->StartRecording(); // start recording 
		dxCommandList->GetCommandList()->ResourceBarrier(startb.size(), startb.data()); // transition resources to copy

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

		dxCommandList->GetCommandList()->ResourceBarrier(endb.size(), endb.data()); // transition resources back to original state
		dxCommandList->Close();

		for (uint32 i = 0; i < m_UploadPages.size(); i++)
			m_UploadPages[i].Clear();

	}
}
