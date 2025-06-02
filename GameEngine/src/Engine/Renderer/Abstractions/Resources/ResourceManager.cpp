#include "pch.h"
#include "ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Utils/Performance.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12ResourceManager.h"
#endif

namespace Engine
{

	// upload page
	UploadPage::UploadPage(uint32 size) :
		m_Size(size), m_UsedMemory(0)
	{
		m_UploadBuffer = UploadBufferResource::Create(size);
		m_BasePointer = m_UploadBuffer->Map();
		m_MemWrightPointer = m_BasePointer;
	}


	UploadPage::~UploadPage()
	{
		m_UploadBuffer->UnMap();
		delete m_UploadBuffer;
		if(Renderer::GetContext())
			Renderer::GetContext()->GetResourceManager()->ScheduleResourceDeletion(m_UploadBuffer);
	}

	void* UploadPage::Map(const void* data, uint32 size)
	{
		CREATE_PROFILE_FUNCTIONI();
		// if the amount of memory requested to upload is greater than the amount of available space return nullptr 
		if ((m_Size - m_UsedMemory) < size)
			return nullptr;

		void* loc = m_MemWrightPointer;
		memcpy(loc, data, size);
		(uint64&)m_MemWrightPointer += size;
		m_UsedMemory += size;

		return loc;
	}

	uint64 UploadPage::GetOffset(const void* data)
	{
		return (uint64)data - (uint64)m_BasePointer;
	}

	void UploadPage::Clear()
	{
		m_UsedMemory = 0;
		m_MemWrightPointer = m_BasePointer;
	}


	ResourceDeletionPool::~ResourceDeletionPool()
	{
		Clear();
	}

	void ResourceDeletionPool::Clear()
	{
		for (uint32 i = 0; i < m_Resources.Count(); i++)
			delete m_Resources[i];

		for (uint32 i = 0; i < m_Descriptors.Count(); i++)
			delete m_Descriptors[i];

		m_Resources.Clear();
		m_Descriptors.Clear();
	}

	UploadPool::~UploadPool()
	{
		for (uint32 i = 0; i < m_UploadPages.Count(); i++)
			m_UploadPages[i]->Clear();
	}

	void UploadPool::UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state)
	{
		CREATE_PROFILE_FUNCTIONI();
		if (dest == nullptr)
		{
			CORE_ERROR("attempting to upload data with null resource");
			__debugbreak();
			return;
		}

		std::lock_guard g(m_UploadMutex);

		UploadPage* page;
		void* loc = nullptr;
		for (uint32 i = 0; i < m_UploadPages.Count(); i++)
		{
			loc = m_UploadPages[i]->Map(data, size);
			if (loc != nullptr)
			{
				page = m_UploadPages[i];
				break;
			}
		}

		if (loc == nullptr)
		{
			CREATE_PROFILE_SCOPEI("Create new upload page");
			m_UploadPages.Push(new UploadPage(size * 2)); // create new page double the size needed
			page = m_UploadPages.Back();
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

	void UploadPool::CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state)
	{
		if (dest == nullptr || src == nullptr)
		{
			CORE_ERROR("attempting to upload data with null resource");
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

	void UploadPool::UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format)
	{

		if (dest == nullptr || src == nullptr)
		{
			CORE_ERROR("attempting to upload data with null resource");
			__debugbreak();
			return;
		}

		std::lock_guard g(m_UploadMutex);
		UploadTextureData uploadData;
		uploadData.destResource = dest;
		uploadData.uploadResource = src;
		uploadData.width = width;
		uploadData.height = height;
		uploadData.numMips = numMips;
		uploadData.state = state;
		uploadData.format = format;

		m_TextureUploadQueue.push(uploadData);
	}

	void UploadPool::RecordBufferCommands(Ref<CommandList> commandlist)
	{
		CREATE_PROFILE_FUNCTIONI();

		// data
		Utils::Vector<ResourceStateObject> startb;
		Utils::Vector<ResourceStateObject> endb;
		Utils::Vector<UploadBufferData> bufferCopys;


		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.size();
		startb.Reserve(numBufferCopys);
		endb.Reserve(numBufferCopys);
		bufferCopys.Reserve(numBufferCopys);

		std::unordered_set<GPUResource*> m_SeenUploads;

		// collect data
		while (!m_BufferUploadQueue.empty())
		{
			UploadBufferData& data = m_BufferUploadQueue.front();

			if (m_SeenUploads.find(data.destResource) == m_SeenUploads.end())
			{
				startb.Push({ data.destResource, ResourceState::CopyDestination });
				endb.Push({ data.destResource, data.state });
				m_SeenUploads.insert(data.destResource);
			}
			bufferCopys.Push(data);

			m_BufferUploadQueue.pop();
		}

		// record commands

		commandlist->StartRecording(); // start recording 
		GPUTimer::BeginEvent(commandlist, "Buffer Upload");
		commandlist->ValidateStates(startb); // transition resources to copy dest

		for (uint32 i = 0; i < numBufferCopys; i++)
		{
			commandlist->CopyBuffer(
				bufferCopys[i].destResource,
				bufferCopys[i].destOffset,
				bufferCopys[i].uploadResource,
				bufferCopys[i].srcOffset,
				bufferCopys[i].size
			);
		}

		commandlist->ValidateStates(endb); // transition resources back to original state
		GPUTimer::EndEvent(commandlist);
		commandlist->Close();

	}

	void UploadPool::RecordTextureCommands(Ref<CommandList> commandlist)
	{
		CREATE_PROFILE_FUNCTIONI();
		// structs
		struct TextureUploadInfo
		{
			GPUResource* dest;
			UploadTextureResource* src;
			uint32 width;
			uint32 height;
			TextureFormat format;
		};

		struct TextureMipInfo
		{
			GPUResource* texture;
			Ref<RWTexture2D> mipTexture;
			uint32 width;
			uint32 height;
			uint32 numMips;
			ResourceState state;
			TextureFormat format;
		};

		struct TextureCopyInfo
		{
			GPUResource* dest;
			GPUResource* src;
		};

		// data
		uint32 numTextureCopys = (uint32)m_TextureUploadQueue.size();

		Utils::Vector<ResourceStateObject> startb;
		Utils::Vector<ResourceStateObject> endb;
		Utils::Vector<TextureUploadInfo> textureUploads;
		Utils::Vector<TextureMipInfo> mipTextures;

		textureUploads.Reserve(numTextureCopys);

		// collect data
		while (!m_TextureUploadQueue.empty())
		{
			UploadTextureData& data = m_TextureUploadQueue.front();

			if (data.numMips > 1)
			{
				// create temp texture to generate the mip levels
				Ref<RWTexture2D> mipTexture = RWTexture2D::Create(data.width, data.height, data.numMips, data.format);
				mipTextures.Push({ data.destResource, mipTexture, data.width, data.height, data.numMips, data.state, data.format });
				//m_TempUplaodResourecs.push_back(mipTexture);

				startb.Push({ data.destResource, ResourceState::CopyDestination });
				startb.Push({ mipTexture->GetResource(), ResourceState::CopyDestination });
				textureUploads.Push({ mipTexture->GetResource(), data.uploadResource, data.width, data.height, data.format });
				endb.Push({ mipTexture->GetResource(), ResourceState::UnorderedResource });
			}
			else
			{
				startb.Push({ data.destResource, ResourceState::CopyDestination });
				textureUploads.Push({ data.destResource, data.uploadResource, data.width, data.height, data.format });
				endb.Push({ data.destResource, data.state });
			}

			m_TextureUploadQueue.pop();
		}


		// start recording 
		commandlist->StartRecording(); // start recording
		GPUTimer::BeginEvent(commandlist, "Texture Upload");
		commandlist->ValidateStates(startb); // transition resources to copy

		GPUTimer::BeginEvent(commandlist, "Upload Textures");
		// copy textures to final texture or mip texture
		for (uint32 i = 0; i < textureUploads.Count(); i++)
		{
			TextureUploadInfo& info = textureUploads[i];
			commandlist->UploadTexture(info.dest, info.src);
		}
		GPUTimer::EndEvent(commandlist);

		commandlist->ValidateStates(endb); // transition resources back to original state


		// generate mip maps
		Utils::Vector<ResourceStateObject> mipBarrier;
		Utils::Vector<ResourceStateObject> textureBarrier;
		Utils::Vector<TextureCopyInfo> mipCopys;

		GPUTimer::BeginEvent(commandlist, "Generate Mips");

		for (auto& mipTexture : mipTextures)
		{
			for (uint32 topMip = 0; topMip < mipTexture.numMips - 1; topMip++)
			{
				Texture2DUAVDescriptorHandle* lastMip = mipTexture.mipTexture->GetUAVDescriptor(topMip);
				Texture2DUAVDescriptorHandle* nextMip = mipTexture.mipTexture->GetUAVDescriptor(topMip + 1);

				// blit the last mip level onto the current mip level
				Ref<Shader> blit = Renderer::GetBlitShader();
				Ref<ComputeShaderPass> blitPass = blit->GetComputePass("Blit");
				commandlist->SetShader(blitPass);
				commandlist->SetRWTexture(blitPass->GetUniformLocation("u_SrcTexture"), lastMip);
				commandlist->SetRWTexture(blitPass->GetUniformLocation("u_DstTexture"), nextMip);
				commandlist->Dispatch(std::max(nextMip->m_Width / 8, 1u) + 1, std::max(nextMip->m_Height / 8, 1u) + 1, 1);

				commandlist->AwaitUAV(mipTexture.mipTexture->GetResource());
			}


			// record barrier for texture and mip
			mipBarrier.Push({ mipTexture.mipTexture->GetResource(), ResourceState::CopySource });
			textureBarrier.Push({ mipTexture.texture, mipTexture.state });

			// copy resource to final texture
			mipCopys.Push({ mipTexture.texture, mipTexture.mipTexture->GetResource() });
		}

		GPUTimer::EndEvent(commandlist);
		GPUTimer::BeginEvent(commandlist, "Copy Mip Textures");

		// copy from mip textures to final texture
		commandlist->ValidateStates(mipBarrier); // transition mips to copy src
		for (uint32 i = 0; i < mipCopys.Count(); i++)
			commandlist->CopyResource(mipCopys[i].dest, mipCopys[i].src);
		commandlist->ValidateStates(textureBarrier); // transition resources back to original state
		GPUTimer::EndEvent(commandlist);
		GPUTimer::EndEvent(commandlist);
		commandlist->Close();
	}

	ResourceManager::ResourceManager()
	{
		m_UploadPool = new UploadPool();
		m_DeletionPool = new ResourceDeletionPool();
	}

	ResourceManager::~ResourceManager()
	{
		delete m_UploadPool;// needs to be destroyed before the heaps
		delete m_DeletionPool; // needs to be destroyed before the heaps

		m_UploadPool = nullptr;
		m_DeletionPool = nullptr;
	}

	ResourceDeletionPool* ResourceManager::SwapDeletionPools()
	{
		ResourceDeletionPool* oldPool = m_DeletionPool;
		m_DeletionPool = new ResourceDeletionPool();
		return oldPool;
	}

	UploadPool* ResourceManager::UploadDataAndSwapPools()
	{
		CREATE_PROFILE_FUNCTIONI();

		std::lock_guard g(m_UploadPool->GetUploadMutex());

		UploadPool* pool = m_UploadPool;
		m_UploadPool = new UploadPool();

		{
			CREATE_PROFILE_SCOPEI("Record commands");
			pool->RecordBufferCommands(m_BufferCopyCommandList);
			pool->RecordTextureCommands(m_TextureCopyCommandList);

		}

		return pool;
	}
}
