#pragma once
#include "Engine/Renderer/Abstractions/Resources/Texture.h"
#include "Engine/Renderer/Abstractions/Resources/UploadBuffer.h"
#include "DirectX12ResourceManager.h"
#include "DirectX12Descriptors.h"
#include "Platform/DirectX12/DX.h"
#include "DirectX12TextureFormat.h"

namespace Engine
{
	class DirectX12Texture2DResource : public Texture2DResource
	{
	public:
		DirectX12Texture2DResource(uint32 width, uint32 height, TextureFormat format, ID3D12Resource* resource);
		DirectX12Texture2DResource(uint32 width, uint32 height, uint32 numMips, TextureFormat format, Math::Vector4 clearColor, TextureType type, MSAASampleCount sampleCount);
		DISABLE_COPY(DirectX12Texture2DResource);
		virtual ~DirectX12Texture2DResource() override;

		ID3D12Resource* GetBuffer() { return m_Buffer; }

		DXGI_FORMAT GetDXGIFormat() { return GetDXGITextureFormat(m_Format); }
		DXGI_FORMAT GetDXGISRVFormat() { return GetDXGISRVTextureFormat(m_Format); }

		virtual void SetData(void* data) override;

	protected:
		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetGPUState(ResourceState state) override;

		virtual void AllocateTransient(class TransientResourceHeap* heap, uint32 offset) override;

	private:
		ID3D12Resource* m_Buffer;
		UploadTextureResource* m_UploadBuffer;

		friend class DirectX12SwapChain;
	};



	class DirectX12Texture2DSRVDescriptorHandle : public Texture2DSRVDescriptorHandle
	{
	public:
		DirectX12Texture2DSRVDescriptorHandle();
		virtual ~DirectX12Texture2DSRVDescriptorHandle() override { m_SRVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_SRVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_SRVHandle.GetIndex(); }
		virtual void Bind(Texture2DResource* resource) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_SRVHandle; }

	private:
		DirectX12DescriptorHandle m_SRVHandle;
	};


	class DirectX12Texture2DUAVDescriptorHandle : public Texture2DUAVDescriptorHandle
	{
	public:
		DirectX12Texture2DUAVDescriptorHandle();
		virtual ~DirectX12Texture2DUAVDescriptorHandle() { m_UAVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_UAVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_UAVHandle.GetIndex(); }
		virtual void Bind(Texture2DResource* resource, uint32 mipSlice, uint32 width, uint32 height) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_UAVHandle; }

	private:
		DirectX12DescriptorHandle m_UAVHandle;
	};


	class DirectX12Texture2DRTVDSVDescriptorHandle : public Texture2DRTVDSVDescriptorHandle
	{
	public:
		DirectX12Texture2DRTVDSVDescriptorHandle(Texture2DResource* resource);
		virtual ~DirectX12Texture2DRTVDSVDescriptorHandle() override { m_RTVDSVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_RTVDSVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_RTVDSVHandle.GetIndex(); }
		virtual void Bind(Texture2DResource* resource) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_RTVDSVHandle; }

	private:
		DirectX12DescriptorHandle m_RTVDSVHandle;
		bool m_DSV;
	};

}
