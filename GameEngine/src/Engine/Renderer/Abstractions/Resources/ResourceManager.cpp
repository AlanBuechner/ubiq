#include "pch.h"
#include "ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Util/Performance.h"
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
		for (uint32 i = 0; i < m_Resources.size(); i++)
			delete m_Resources[i];

		for (uint32 i = 0; i < m_Descriptors.size(); i++)
			delete m_Descriptors[i];

		m_Resources.clear();
		m_Descriptors.clear();
	}

	ResourceManager::ResourceManager()
	{
		m_DeletionPool = new ResourceDeletionPool();
	}

	ResourceManager::~ResourceManager()
	{}


	ResourceDeletionPool* ResourceManager::CreateNewDeletionPool()
	{
		ResourceDeletionPool* pool = m_DeletionPool;
		m_DeletionPool = new ResourceDeletionPool();
		return pool;
	}

	void ResourceManager::UploadData()
	{
		CREATE_PROFILE_FUNCTIONI();

		InstrumentationTimer timer = CREATE_PROFILEI();
		std::lock_guard g(m_UploadMutex);

		timer.Start("Record commands");
		RecordBufferCommands();
		RecordTextureCommands();
		timer.End();

		//m_CommandQueue->Submit(m_BufferCopyCommandList);
		//m_CommandQueue->Submit(m_TextureCopyCommandList);
		//m_CommandQueue->Build();
		//m_CommandQueue->Execute();
		//m_CommandQueue->Await();

	}

	void ResourceManager::Clean()
	{
		// clear upload pages
		for (uint32 i = 0; i < m_UploadPages.size(); i++)
			m_UploadPages[i]->Clear();
	}

	void ResourceManager::UploadBuffer(GPUResource* dest, const void* data, uint32 size, ResourceState state)
	{
		UploadBufferRegion(dest, 0, data, size, state);
	}

	void ResourceManager::UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state)
	{
		if (dest == nullptr)
		{
			CORE_ERROR("atempting to upload data with null resource");
			__debugbreak();
			return;
		}

		std::lock_guard g(m_UploadMutex);

		UploadPage* page;
		void* loc = nullptr;
		for (uint32 i = 0; i < m_UploadPages.size(); i++)
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
			m_UploadPages.push_back(new UploadPage(size * 2)); // create new page double the size needed
			page = m_UploadPages.back();
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

	void ResourceManager::CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state)
	{
		if (dest == nullptr || src == nullptr)
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

	void ResourceManager::UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format)
	{

		if (dest == nullptr || src == nullptr)
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
		uploadData.numMips = numMips;
		uploadData.state = state;
		uploadData.format = format;

		m_TextureUploadQueue.push(uploadData);
	}

	void ResourceManager::RecordBufferCommands()
	{
		CREATE_PROFILE_FUNCTIONI();

		// data
		std::vector<ResourceStateObject> startb;
		std::vector<ResourceStateObject> endb;
		std::vector<UploadBufferData> bufferCopys;


		uint32 numBufferCopys = (uint32)m_BufferUploadQueue.size();
		startb.reserve(numBufferCopys);
		endb.reserve(numBufferCopys);
		bufferCopys.reserve(numBufferCopys);

		std::unordered_set<GPUResource*> m_SeenUploads;

		// collect data
		while (!m_BufferUploadQueue.empty())
		{
			UploadBufferData& data = m_BufferUploadQueue.front();

			if (m_SeenUploads.find(data.destResource) == m_SeenUploads.end())
			{
				startb.push_back({ data.destResource, ResourceState::CopyDestination });
				endb.push_back({ data.destResource, data.state });
				m_SeenUploads.insert(data.destResource);
			}
			bufferCopys.push_back(data);

			m_BufferUploadQueue.pop();
		}

		// record commands

		m_BufferCopyCommandList->StartRecording(); // start recording 
		GPUTimer::BeginEvent(m_BufferCopyCommandList, "Buffer Upload");
		m_BufferCopyCommandList->ValidateStates(startb); // transition resources to copy dest

		for (uint32 i = 0; i < numBufferCopys; i++)
		{
			m_BufferCopyCommandList->CopyBuffer(
				bufferCopys[i].destResource,
				bufferCopys[i].destOffset,
				bufferCopys[i].uploadResource,
				bufferCopys[i].srcOffset,
				bufferCopys[i].size
			);
		}

		m_BufferCopyCommandList->ValidateStates(endb); // transition resources back to original state
		GPUTimer::EndEvent(m_BufferCopyCommandList);
		m_BufferCopyCommandList->Close();

	}

	void ResourceManager::RecordTextureCommands()
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

		std::vector<ResourceStateObject> startb;
		std::vector<ResourceStateObject> endb;
		std::vector<TextureUploadInfo> textureUploads;
		std::vector<TextureMipInfo> mipTextures;

		textureUploads.reserve(numTextureCopys);

		// collect data
		while (!m_TextureUploadQueue.empty())
		{
			UploadTextureData& data = m_TextureUploadQueue.front();

			if (data.numMips > 1)
			{
				// create temp texture to generate the mip levels
				Ref<RWTexture2D> mipTexture = RWTexture2D::Create(data.width, data.height, data.numMips, data.format);
				mipTextures.push_back({ data.destResource, mipTexture, data.width, data.height, data.numMips, data.state, data.format });
				//m_TempUplaodResourecs.Push(mipTexture);

				startb.push_back({ data.destResource, ResourceState::CopyDestination });
				startb.push_back({ mipTexture->GetResource(), ResourceState::CopyDestination });
				textureUploads.push_back({ mipTexture->GetResource(), data.uploadResource, data.width, data.height, data.format });
				endb.push_back({ mipTexture->GetResource(), ResourceState::UnorderedResource });
			}
			else
			{
				startb.push_back({ data.destResource, ResourceState::CopyDestination });
				textureUploads.push_back({ data.destResource, data.uploadResource, data.width, data.height, data.format });
				endb.push_back({ data.destResource, data.state });
			}

			m_TextureUploadQueue.pop();
		}


		// start recording 
		m_TextureCopyCommandList->StartRecording(); // start recording
		GPUTimer::BeginEvent(m_TextureCopyCommandList, "Texture Upload");
		m_TextureCopyCommandList->ValidateStates(startb); // transition resources to copy

		GPUTimer::BeginEvent(m_TextureCopyCommandList, "Upload Textures");
		// copy textures to final texture or mip texture
		for (uint32 i = 0; i < textureUploads.size(); i++)
		{
			TextureUploadInfo& info = textureUploads[i];
			m_TextureCopyCommandList->UploadTexture(info.dest, info.src);
		}
		GPUTimer::EndEvent(m_TextureCopyCommandList);

		m_TextureCopyCommandList->ValidateStates(endb); // transition resources back to original state


		// generate mip maps
		std::vector<ResourceStateObject> mipBarrier;
		std::vector<ResourceStateObject> textureBarrier;
		std::vector<TextureCopyInfo> mipCopys;

		GPUTimer::BeginEvent(m_TextureCopyCommandList, "Generate Mips");

		for (auto& mipTexture : mipTextures)
		{
			for (uint32 topMip = 0; topMip < mipTexture.numMips - 1; topMip++)
			{
				Texture2DUAVDescriptorHandle* lastMip = mipTexture.mipTexture->GetUAVDescriptor(topMip);
				Texture2DUAVDescriptorHandle* nextMip = mipTexture.mipTexture->GetUAVDescriptor(topMip + 1);

				// blit the last mip level onto the current mip level
				Ref<Shader> blit = Renderer::GetBlitShader();
				Ref<ComputeShaderPass> blitPass = blit->GetComputePass("Blit");
				m_TextureCopyCommandList->SetShader(blitPass);
				m_TextureCopyCommandList->SetRWTexture(blitPass->GetUniformLocation("SrcTexture"), lastMip);
				m_TextureCopyCommandList->SetRWTexture(blitPass->GetUniformLocation("DstTexture"), nextMip);
				m_TextureCopyCommandList->Dispatch(std::max(nextMip->m_Width / 8, 1u) + 1, std::max(nextMip->m_Height / 8, 1u) + 1, 1);

				m_TextureCopyCommandList->AwaitUAV(mipTexture.mipTexture->GetResource());
			}


			// record barrier for texture and mip
			mipBarrier.push_back({ mipTexture.mipTexture->GetResource(), ResourceState::CopySource });
			textureBarrier.push_back({ mipTexture.texture, mipTexture.state });

			// copy resource to final texture
			mipCopys.push_back({ mipTexture.texture, mipTexture.mipTexture->GetResource() });
		}

		GPUTimer::EndEvent(m_TextureCopyCommandList);
		GPUTimer::BeginEvent(m_TextureCopyCommandList, "Copy Mip Textures");

		// copy from mip textures to final texture
		m_TextureCopyCommandList->ValidateStates(mipBarrier); // transition mips to copy src
		for (uint32 i = 0; i < mipCopys.size(); i++)
			m_TextureCopyCommandList->CopyResource(mipCopys[i].dest, mipCopys[i].src);
		m_TextureCopyCommandList->ValidateStates(textureBarrier); // transition resources back to original state
		GPUTimer::EndEvent(m_TextureCopyCommandList);
		GPUTimer::EndEvent(m_TextureCopyCommandList);
		m_TextureCopyCommandList->Close();
	}

}
