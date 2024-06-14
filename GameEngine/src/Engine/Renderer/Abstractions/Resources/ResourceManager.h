#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Flag.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/CommandQueue.h"
#include "Engine/Renderer/Abstractions/Resources/Descriptor.h"
#include "Engine/Renderer/Abstractions/Resources/ResourceState.h"
#include "UploadBuffer.h"

#include <thread>
#include <queue>
#include <mutex>

namespace Engine
{

	class UploadPage
	{
	public:
		UploadPage(uint32 size);
		~UploadPage();

		void* Map(const void* data, uint32 size);
		uint64 GetOffset(const void* data);
		UploadBufferResource* GetResource() { return m_UploadBuffer; }
		void Clear();

	private:
		UploadBufferResource* m_UploadBuffer;
		uint32 m_Size = 0;
		uint32 m_UsedMemory = 0;
		void* m_MemWrightPointer = nullptr;
		void* m_BasePointer = nullptr;
	};

	class ResourceDeletionPool
	{
	public:
		~ResourceDeletionPool();
		void Clear();

		Utils::Vector<GPUResource*> m_Resources;
		Utils::Vector<Descriptor*> m_Descriptors;
	};

	struct UploadBufferData
	{
		GPUResource* destResource;
		GPUResource* uploadResource;
		uint64 destOffset;
		uint64 srcOffset;
		uint32 size;
		ResourceState state;
	};

	struct UploadTextureData
	{
		GPUResource* destResource;
		UploadTextureResource* uploadResource;
		uint32 width;
		uint32 height;
		uint32 numMips;
		ResourceState state;
		TextureFormat format;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		~ResourceManager();

		ResourceDeletionPool* CreateNewDeletionPool();

		void UploadData();
		void Clean();

		void UploadBuffer(GPUResource* dest, const void* data, uint32 size, ResourceState state);
		void UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state);
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state);

		void UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format);

		Utils::Vector<Ref<CommandList>> GetUploadCommandLists() { return { m_BufferCopyCommandList, m_TextureCopyCommandList }; }

		void ScheduleResourceDeletion(GPUResource* resource) { m_DeletionPool->m_Resources.Push(resource); }
		void ScheduleHandleDeletion(Descriptor* descriptor) { m_DeletionPool->m_Descriptors.Push(descriptor); }
		
	private:
		void RecordBufferCommands();
		void RecordTextureCommands();

	protected:
		ResourceDeletionPool* m_DeletionPool;
		Utils::Vector<UploadPage*> m_UploadPages;

		std::mutex m_UploadMutex;

		Ref<CommandList> m_BufferCopyCommandList;
		Ref<CommandList> m_TextureCopyCommandList;

		std::queue<UploadBufferData> m_BufferUploadQueue;
		std::queue<UploadTextureData> m_TextureUploadQueue;

	};
}
