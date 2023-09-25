#pragma once
#include "Engine/Renderer/ResourceManager.h"
#include "DirectX12Descriptors.h"
#include "DX.h"
#include <queue>
#include <mutex>

#include "DirectX12CommandQueue.h"
#include "DirectX12CommandList.h"

namespace Engine
{
	class DirectX12Context;
}

namespace Engine
{

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
		struct UploadBufferData
		{
			wrl::ComPtr<ID3D12Resource> destResource;
			wrl::ComPtr<ID3D12Resource> uploadResource;
			uint64 destOffset;
			uint64 srcOffset;
			uint32 size;
			D3D12_RESOURCE_STATES state;
		};

		struct UploadTextureData
		{
			wrl::ComPtr<ID3D12Resource> destResource;
			wrl::ComPtr<ID3D12Resource> uploadResource;
			uint32 width;
			uint32 height;
			uint32 pitch;
			uint32 numMips;
			D3D12_RESOURCE_STATES state;
			DXGI_FORMAT format;
		};

	public:
		static Ref<DirectX12DescriptorHeap> s_SRVHeap;
		static Ref<DirectX12DescriptorHeap> s_RTVHeap;
		static Ref<DirectX12DescriptorHeap> s_DSVHeap;
		static Ref<DirectX12DescriptorHeap> s_SamplerHeap;

	public:

		DirectX12ResourceManager();
		~DirectX12ResourceManager();

		static wrl::ComPtr<ID3D12Resource> CreateUploadTexture2D(DXGI_FORMAT format, uint32 width, uint32 height);
		static ID3D12Resource* CreateUploadBuffer(uint32 size);
		void UploadBuffer(wrl::ComPtr<ID3D12Resource> dest, const void* data, uint32 size, D3D12_RESOURCE_STATES state);
		void UploadBufferRegion(wrl::ComPtr<ID3D12Resource> dest, uint64 offset, const void* data, uint32 size, D3D12_RESOURCE_STATES state);
		void CopyBuffer(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 size, D3D12_RESOURCE_STATES state);

		void UploadTexture(wrl::ComPtr<ID3D12Resource> dest, wrl::ComPtr<ID3D12Resource> src, uint32 width, uint32 height, uint32 pitch, uint32 numMips, D3D12_RESOURCE_STATES state, DXGI_FORMAT format);

	private:
		virtual void UploadData() override;
		virtual void Clean() override;
		virtual std::vector<Ref<CommandList>> GetUploadCommandLists() override { return { m_BufferCopyCommandList, m_TextureCopyCommandList }; }

		void RecordBufferCommands(Ref<DirectX12Context> context);
		void RecordTextureCommands(Ref<DirectX12Context> context);

	private:
		Ref<DirectX12CommandList> m_BufferCopyCommandList;
		Ref<DirectX12CommandList> m_TextureCopyCommandList;

		Ref<DirectX12DescriptorHeap> m_UploadDescriptors;
		std::vector<wrl::ComPtr<ID3D12Resource>> m_TempUplaodResourecs;

		std::queue<UploadBufferData> m_BufferUploadQueue;
		std::queue<UploadTextureData> m_TextureUploadQueue;
		std::vector<DirectX12UploadPage> m_UploadPages;

		std::mutex m_UploadMutex;
	};
}
