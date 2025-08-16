#include "pch.h"
#include "ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Utils/Performance.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "Engine/Renderer/Renderer.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/Resources/DirectX12ResourceManager.h"
#endif

uint32 Engine::ResourceManager::s_CachedUploadPageSize = MEM_MiB(5);
Utils::Vector<Engine::UploadPage*> Engine::ResourceManager::s_CachedUploadPages;

namespace Engine
{

	// upload page
	UploadPage::UploadPage(uint32 size) :
		m_Size(size), m_UsedMemory(0)
	{
		CREATE_PROFILE_SCOPEI("Create Upload Buffer");
		ANOTATE_PROFILEI("Size: " + std::to_string(size));
		m_UploadBuffer = UploadBufferResource::Create(size);
		m_BasePointer = m_UploadBuffer->Map();
		m_MemWrightPointer = m_BasePointer;
	}


	UploadPage::~UploadPage()
	{
		CREATE_PROFILE_FUNCTIONI();
		m_UploadBuffer->UnMap();
		delete m_UploadBuffer;
	}

	void* UploadPage::Map(const void* data, uint32 size)
	{
		CREATE_PROFILE_FUNCTIONI();
		// if the amount of memory requested to upload is greater than the amount of available space return nullptr 
		if ((m_Size - m_UsedMemory) < size)
		{
			ANOTATE_PROFILEI("Page map failed");
			return nullptr;
		}

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
		CREATE_PROFILE_FUNCTIONI();
		for (uint32 i = 0; i < m_Resources.Count(); i++)
			delete m_Resources[i];

		for (uint32 i = 0; i < m_Descriptors.Count(); i++)
			delete m_Descriptors[i];

		m_Resources.Clear();
		m_Descriptors.Clear();
	}

	UploadPool::~UploadPool()
	{
		CREATE_PROFILE_FUNCTIONI();
		ResourceManager::FreeUploadPage(m_UploadPage);
	}

	void UploadPool::SubmitBuffer(GPUResource* dest, uint32 destOffset, void* data, uint32 size, ResourceState state)
	{
		CREATE_PROFILE_FUNCTIONI();
		ANOTATE_PROFILEI("Upload Size: " + std::to_string(size));

		CORE_ASSERT(data != nullptr, "Data cant not be nullptr");
		CORE_ASSERT(size != 0, "Can not upload buffer with size of 0");

		UploadBufferData uploadData;
		uploadData.size = size;
		uploadData.destOffset = destOffset;
		uploadData.srcOffset = 0;
		uploadData.destResource = dest;
		uploadData.uploadResource = nullptr;
		uploadData.data = data;
		if (state == ResourceState::Unknown)
			uploadData.state = dest->GetDefultState();
		else
			uploadData.state = state;

		std::lock_guard g(m_UploadMutex);
		m_BufferUploadQueue.Push(uploadData);
	}

	void* UploadPool::LockBuffer(uint32 size)
	{
		CORE_ASSERT(size != 0, "Can not lock upload buffer with size of 0");
		return new byte[size]; // create new buffer
	}

	void UploadPool::UnlockBuffer(GPUResource* dest, uint32 destOffset, void* data, uint32 size, ResourceState state)
	{
		SubmitBuffer(dest, destOffset, data, size, state);
	}

	void UploadPool::UploadBufferRegion(GPUResource* dest, uint32 destOffset, const void* data, uint32 size, ResourceState state)
	{
		byte* lockedBuffer = (byte*)LockBuffer(size);
		memcpy(lockedBuffer, data, size);
		UnlockBuffer(dest, destOffset, lockedBuffer, size, state);
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

		m_BufferUploadQueue.Push(uploadData);
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

	void UploadPool::RecoredUploadCommands(Ref<CPUCommandList> commandList)
	{
		CREATE_PROFILE_SCOPEI("Record Upload Commands");
		commandList->StartRecording();
		RecordBufferCommands(commandList);
		RecordTextureCommands(commandList);
		commandList->StopRecording();
	}

	void UploadPool::RecordBufferCommands(Ref<CPUCommandList> commandlist)
	{
		CREATE_PROFILE_FUNCTIONI();

		// find needed buffer size
		uint32 uploadBufferSize = 0;
		for (uint32 i = 0; i < m_BufferUploadQueue.Count(); i++)
		{
			UploadBufferData& data = m_BufferUploadQueue[i];
			if (data.data != nullptr)
				uploadBufferSize += data.size;
		}

		// create new upload buffer
		if(uploadBufferSize != 0)
			m_UploadPage = ResourceManager::GetUploadPage(uploadBufferSize);

		// data
		Utils::Vector<ResourceStateObject> startb;
		Utils::Vector<ResourceStateObject> endb;
		Utils::Vector<UploadBufferData> bufferCopys;

		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.Count();
		startb.Reserve(numBufferCopys);
		endb.Reserve(numBufferCopys);
		bufferCopys.Reserve(numBufferCopys);

		std::unordered_set<GPUResource*> m_SeenUploads;

		// collect data
		for(uint32 i = 0; i < m_BufferUploadQueue.Count(); i++)
		{
			UploadBufferData& data = m_BufferUploadQueue[i];

			// copy data to upload resource if needed
			if (m_UploadPage != nullptr)
			{
				void* dataLoc = m_UploadPage->Map(data.data, data.size); // copy data into resource
				data.uploadResource = m_UploadPage->GetResource();
				data.srcOffset = m_UploadPage->GetOffset(dataLoc);
			}
			// always delete old data
			if (data.data != nullptr)
			{
				// delete old data
				delete data.data;
				data.data = nullptr;
			}

			// collect resource states
			if (m_SeenUploads.find(data.destResource) == m_SeenUploads.end())
			{
				startb.Push({ data.destResource, ResourceState::CopyDestination });
				endb.Push({ data.destResource, data.state });
				m_SeenUploads.insert(data.destResource);
			}
			bufferCopys.Push(data);
		}

		// record commands

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

	}

	void UploadPool::RecordTextureCommands(Ref<CPUCommandList> commandlist)
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
				commandlist->SetRWTexture("u_SrcTexture", lastMip);
				commandlist->SetRWTexture("u_DstTexture", nextMip);
				commandlist->DispatchThreads(nextMip->m_Width, nextMip->m_Height + 1, 1);

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
	}

	ResourceManager::ResourceManager()
	{}

	ResourceManager::~ResourceManager()
	{}

	ResourceDeletionPool* ResourceManager::GetDeletionPool()
	{
		return Renderer::GetFrameContext()->m_DeletionPool;
	}

	UploadPool* ResourceManager::GetUploadPool()
	{
		return Renderer::GetFrameContext()->m_UploadPool;
	}

	UploadPage* ResourceManager::GetUploadPage(uint32 size)
	{
		// if size is larger than cash size create new page
		if (size > s_CachedUploadPageSize)
		{
			CORE_WARN("Createing upload page larger than cashed page size. Try increasing cashed page size");
			return new UploadPage(size);
		}

		// find page page with current cash size delete others (needed if cash size is updated mid game)
		while (!s_CachedUploadPages.Empty())
		{
			UploadPage* page = s_CachedUploadPages.Pop();
			if (page->GetSize() != s_CachedUploadPageSize)
				delete page;
			else
				return page;
		}

		// if no pages create new one
		return new UploadPage(s_CachedUploadPageSize);
	}

	void ResourceManager::FreeUploadPage(UploadPage* page)
	{
		if (page == nullptr)
			return;

		s_CachedUploadPages.Push(page);
	}

}
