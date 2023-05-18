#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Resources/StructuredBuffer.h"
#include "DirectX12ResourceManager.h"
#include "DX.h"

namespace Engine
{
	class DirectX12StructuredBufferResource : public StructuredBufferResource
	{
	public:
		DirectX12StructuredBufferResource(uint32 stride, uint32 count);
		~DirectX12StructuredBufferResource();

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }

		DirectX12DescriptorHandle& GetSRVHandle() { return m_SRVHandle; }
		DirectX12DescriptorHandle& GetUAVHandle() { return m_UAVHandle; }

	private:
		void Resize(uint32 count);
		void SetData(const void* data, uint32 count = 1, uint32 start = 0);

		void CreateSRVHandle();
		void CreateUAVHandle();

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_SRVHandle;
		DirectX12DescriptorHandle m_UAVHandle;

		friend class DirectX12StructuredBuffer;
		friend class DirectX12RWStructuredBuffer;
	};


	class DirectX12StructuredBuffer : public StructuredBuffer
	{
	public:
		DirectX12StructuredBuffer(uint32 stride, uint32 count);
		DirectX12StructuredBuffer(Ref<StructuredBufferResource> resource);

		virtual void Resize(uint32 count) override { m_Resource->Resize(count); }

		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) { m_Resource->SetData(data, count, start); }

		virtual uint32 GetDescriptorLocation() const override { return m_Resource->m_SRVHandle.GetIndex(); }

		virtual Ref<StructuredBufferResource> GetResource() { return m_Resource; }
		virtual Ref<DirectX12StructuredBufferResource> GetDXResource() { return m_Resource; }

	private:
		Ref<DirectX12StructuredBufferResource> m_Resource;

	};




	class DirectX12RWStructuredBuffer : public RWStructuredBuffer
	{
	public:
		DirectX12RWStructuredBuffer(uint32 stride, uint32 count);
		DirectX12RWStructuredBuffer(Ref<StructuredBufferResource> resource);

		virtual void Resize(uint32 count) override { m_Resource->Resize(count); }

		virtual void SetData(const void* data, uint32 count = 1, uint32 start = 0) { m_Resource->SetData(data, count, start); }

		virtual uint32 GetDescriptorLocation() const override { return m_Resource->m_SRVHandle.GetIndex(); }

		virtual Ref<StructuredBufferResource> GetResource() { return m_Resource; }
		virtual Ref<DirectX12StructuredBufferResource> GetDXResource() { return m_Resource; }

	private:
		Ref<DirectX12StructuredBufferResource> m_Resource;

	};




}
