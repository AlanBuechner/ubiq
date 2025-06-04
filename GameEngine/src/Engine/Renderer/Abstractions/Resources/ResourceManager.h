#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Threading/Flag.h"
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

	class ResourceDeletionPool
	{
	public:
		~ResourceDeletionPool();
		void Clear();

		Utils::Vector<GPUResource*> m_Resources;
		Utils::Vector<Descriptor*> m_Descriptors;
	};

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

	class UploadPool
	{
	public:
		~UploadPool();

		void UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state);
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state);

		void UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format);

		void RecoredUploadCommands(Ref<CommandList> commandList);

		void RecordBufferCommands(Ref<CommandList> commandList);
		void RecordTextureCommands(Ref<CommandList> commandList);

		std::mutex& GetUploadMutex() { return m_UploadMutex; }

	private:
		Utils::Vector<UploadPage*> m_UploadPages;

		std::queue<UploadBufferData> m_BufferUploadQueue;
		std::queue<UploadTextureData> m_TextureUploadQueue;
		std::mutex m_UploadMutex;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();

		ResourceDeletionPool* GetDeletionPool() { return m_DeletionPool; }
		ResourceDeletionPool* SwapDeletionPools();

		// records upload commands and create new upload pool
		// returns upload pool to be cached until commandlists have been executed
		UploadPool* SwapPools();

		void UploadBuffer(GPUResource* dest, const void* data, uint32 size, ResourceState state) { UploadBufferRegion(dest, 0, data, size, state); }
		void UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state) { m_UploadPool->UploadBufferRegion(dest, offset, data, size, state); }
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state) { m_UploadPool->CopyBuffer(dest, src, size, state); }

		void UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format)
			{ m_UploadPool->UploadTexture(dest, src, width, height, numMips, state, format); }

		void ScheduleResourceDeletion(GPUResource* resource) { m_DeletionPool->m_Resources.Push(resource); }
		void ScheduleHandleDeletion(Descriptor* descriptor) { m_DeletionPool->m_Descriptors.Push(descriptor); }
		
	private:

	protected:
		ResourceDeletionPool* m_DeletionPool;
		UploadPool* m_UploadPool;

	};
}
