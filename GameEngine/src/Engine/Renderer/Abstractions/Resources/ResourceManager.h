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
		GPUResource* destResource = nullptr;
		/* used if data is to be copied into intermediate buffer on render thread before upload. nullptr if copying from existing resource */
		void* data = nullptr;
		/* used if data is to be copied from existing resource. nullptr if using render thread upload buffer */
		GPUResource* uploadResource = nullptr;
		uint64 destOffset = 0;
		uint64 srcOffset = 0; // 0 if using data is not nullptr
		uint32 size = 0;
		ResourceState state = ResourceState::Unknown;
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

		void SubmitBuffer(GPUResource* dest, uint32 destOffset, void* data, uint32 size, ResourceState state = ResourceState::Unknown);

		void* LockBuffer(uint32 size);
		void UnlockBuffer(GPUResource* dest, uint32 destOffset, void* data, uint32 size, ResourceState state = ResourceState::Unknown);

		void UploadBufferRegion(GPUResource* dest, uint32 destOffset, const void* data, uint32 size, ResourceState state = ResourceState::Unknown);
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state);

		void UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format);

		void RecoredUploadCommands(Ref<CPUCommandList> commandList);

		void RecordBufferCommands(Ref<CPUCommandList> commandList);
		void RecordTextureCommands(Ref<CPUCommandList> commandList);

		std::mutex& GetUploadMutex() { return m_UploadMutex; }

	private:
		UploadPage* m_UploadPage;

		Utils::Vector<UploadBufferData> m_BufferUploadQueue; // data needs to be deleted when dequeued
		std::queue<UploadTextureData> m_TextureUploadQueue;
		std::mutex m_UploadMutex;
	};

	class ResourceManager
	{
	public:
		ResourceManager();
		virtual ~ResourceManager();

		ResourceDeletionPool* GetDeletionPool();
		UploadPool* GetUploadPool();

		void UploadBuffer(GPUResource* dest, const void* data, uint32 size, ResourceState state = ResourceState::Unknown) { UploadBufferRegion(dest, 0, data, size, state); }
		void UploadBufferRegion(GPUResource* dest, uint64 offset, const void* data, uint32 size, ResourceState state = ResourceState::Unknown) 
			{ GetUploadPool()->UploadBufferRegion(dest, offset, data, size, state); }
		void CopyBuffer(GPUResource* dest, GPUResource* src, uint32 size, ResourceState state) { GetUploadPool()->CopyBuffer(dest, src, size, state); }

		void UploadTexture(GPUResource* dest, UploadTextureResource* src, uint32 width, uint32 height, uint32 numMips, ResourceState state, TextureFormat format)
			{ GetUploadPool()->UploadTexture(dest, src, width, height, numMips, state, format); }

		void ScheduleResourceDeletion(GPUResource* resource) { GetDeletionPool()->m_Resources.Push(resource); }
		void ScheduleHandleDeletion(Descriptor* descriptor) { GetDeletionPool()->m_Descriptors.Push(descriptor); }
	};
}
