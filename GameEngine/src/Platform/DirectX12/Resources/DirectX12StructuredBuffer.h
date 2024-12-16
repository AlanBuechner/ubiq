#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Abstractions/Resources/StructuredBuffer.h"
#include "DirectX12ResourceManager.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	
	class DirectX12StructuredBufferResource : public StructuredBufferResource
	{
	public:
		DirectX12StructuredBufferResource(uint32 count, uint32 stride, ParentType parantType);
		virtual ~DirectX12StructuredBufferResource() override;

		ID3D12Resource* GetBuffer() { return m_Buffer; }

		virtual void SetData(const void* data, uint32 count, uint32 start) override;

	private:
		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetState(ResourceState state) override;

	private:
		ID3D12Resource* m_Buffer;
	};




	class DirectX12StructuredBufferSRVDescriptorHandle : public StructuredBufferSRVDescriptorHandle
	{
	public:
		DirectX12StructuredBufferSRVDescriptorHandle();
		virtual ~DirectX12StructuredBufferSRVDescriptorHandle() override { m_SRVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_SRVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_SRVHandle.GetIndex(); }
		virtual void Bind(StructuredBufferResource* resource) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_SRVHandle; }

	private:
		DirectX12DescriptorHandle m_SRVHandle;
	};

	class DirectX12StructuredBufferUAVDescriptorHandle : public StructuredBufferUAVDescriptorHandle
	{
	public:
		DirectX12StructuredBufferUAVDescriptorHandle();
		virtual ~DirectX12StructuredBufferUAVDescriptorHandle() override { m_UAVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_UAVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_UAVHandle.GetIndex(); }
		virtual void Bind(StructuredBufferResource* resource) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_UAVHandle; }

	private:
		DirectX12DescriptorHandle m_UAVHandle;
	};

}
