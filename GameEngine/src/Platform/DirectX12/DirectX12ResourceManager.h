#pragma once
#include "Engine/Renderer/ResourceManager.h"
#include "DirectX12Descriptors.h"
#include "DX.h"
#include <queue>
#include <mutex>

namespace Engine
{
	class DirectX12Context;
}

namespace Engine
{
	class DirectX12ResourceDeletionPool : public ResourceDeletionPool
	{
	public:
		virtual void Clear() override;
		void AddResource(wrl::ComPtr<ID3D12Resource> resource);
	public:
		std::vector<wrl::ComPtr<ID3D12Resource>> m_Pool;
	};

	class DirectX12UploadPage
	{
	public:
		DirectX12UploadPage(uint32 size);
		~DirectX12UploadPage();

		void* Map(const void* data, uint32 size);
		uint64 GetOffset(const void* data);
		wrl::ComPtr<ID3D12Resource> GetResource() { return m_UploadBuffer; }
		void Clear();

	private:
		wrl::ComPtr<ID3D12Resource> m_UploadBuffer;
		uint32 m_Size = 0;
		uint32 m_UsedMemory = 0;
		void* m_MemWrightPointer = nullptr;
		void* m_BasePointer = nullptr;
	};

	class DirectX12ResourceManager : public ResourceManager
	{
	public:
		struct UploadData
		{
			wrl::ComPtr<ID3D12Resource> destResource;
			wrl::ComPtr<ID3D12Resource> uploadResource;
			uint64 destOffset;
			uint64 srcOffset;
			uint32 size;
			D3D12_RESOURCE_STATES state;
		};

	public:
		static Ref<DirectX12DescriptorHeap> s_SRVHeap;
		static Ref<DirectX12DescriptorHeap> s_RTVHeap;
		static Ref<DirectX12DescriptorHeap> s_DSVHeap;
		static Ref<DirectX12DescriptorHeap> s_SamplerHeap;

	public:

		DirectX12ResourceManager();
		~DirectX12ResourceManager();

		virtual Ref<ResourceDeletionPool> CreateNewDeletionPool() override;

		static wrl::ComPtr<ID3D12Resource> CreateUploadTexture2D(DXGI_FORMAT format, uint32 width, uint32 height);
		static wrl::ComPtr<ID3D12Resource> CreateUploadBuffer(uint32 size);
		void UploadBuffer(wrl::ComPtr<ID3D12Resource> dest, const void* data, uint32 size, D3D12_RESOURCE_STATES state);
		void UploadBufferRegion(wrl::ComPtr<ID3D12Resource> dest, uint64 offset, const void* data, uint32 size, D3D12_RESOURCE_STATES state);

		void ScheduleResourceDeletion(wrl::ComPtr<ID3D12Resource> resource) { m_DeletionPool->AddResource(resource); }

	private:
		virtual void RecordCommands(Ref<CommandList> commandList) override;

	private:
		std::queue<UploadData> m_UploadQueue;
		std::vector<DirectX12UploadPage> m_UploadPages;

		Ref<DirectX12ResourceDeletionPool> m_DeletionPool;

		std::mutex m_UploadMutex;
	};
}