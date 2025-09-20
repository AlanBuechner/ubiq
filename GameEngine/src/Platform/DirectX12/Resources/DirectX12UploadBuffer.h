#pragma once
#include "Engine/Renderer/Abstractions/Resources/UploadBuffer.h"
#include "Engine/Renderer/Abstractions/Resources/TextureFormat.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	class DirectX12UploadBufferResource : public UploadBufferResource
	{
	public:
		DirectX12UploadBufferResource(uint32 size);
		virtual ~DirectX12UploadBufferResource();

		virtual void* Map() override;
		virtual void UnMap() override;

		ID3D12Resource* GetBuffer() { return m_Buffer; }

	protected:
		virtual void* GetGPUResourcePointer() const override { return m_Buffer; }
		virtual uint32 GetGPUState(ResourceState state) const override;

		virtual void AllocateTransient(class TransientResourceHeap* heap, uint32 offset) override;

	private:
		ID3D12Resource* m_Buffer;
	};







	class DirectX12UploadTextureResource : public UploadTextureResource
	{
	public:
		DirectX12UploadTextureResource(uint32 width, uint32 height, uint32 mips, TextureFormat format);
		virtual ~DirectX12UploadTextureResource();

		virtual void* Map() override;
		virtual void UnMap() override;

		ID3D12Resource* GetBuffer() { return m_Buffer; }

	protected:
		virtual void* GetGPUResourcePointer() const override { return m_Buffer; }
		virtual uint32 GetGPUState(ResourceState state) const override;

		virtual void AllocateTransient(class TransientResourceHeap* heap, uint32 offset) override;

	private:
		ID3D12Resource* m_Buffer;
	};


}
