#pragma once
#include "Engine/Renderer/Resources/ConstantBuffer.h"
#include "DirectX12Descriptors.h"
#include "DX.h"


namespace Engine
{
	struct DirectX12ConstantBufferResource : public ConstantBufferResource
	{
	public:
		DirectX12ConstantBufferResource(uint32 size);
		~DirectX12ConstantBufferResource();

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }

		const DirectX12DescriptorHandle& GetSRVHandle() { return m_SRVHandle; }
		const DirectX12DescriptorHandle& GetUAVHandle() { return m_UAVHandle; }


	private:
		void SetData(const void* data);

		void CreateSRVHandle();
		void CreateUAVHandle();

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_SRVHandle;
		DirectX12DescriptorHandle m_UAVHandle;

		friend class DirectX12ConstantBuffer;
		friend class DirectX12RWConstantBuffer;
	};


	class DirectX12ConstantBuffer : public ConstantBuffer
	{
	public:
		DirectX12ConstantBuffer(uint32 size);
		DirectX12ConstantBuffer(Ref<ConstantBufferResource> resource);

		virtual void SetData(const void* data) override;

		virtual uint32 GetDescriptorLocation() override { return m_Resource->m_SRVHandle.GetIndex(); }

		virtual Ref<ConstantBufferResource> GetResource() override { return m_Resource; }

		Ref<DirectX12ConstantBufferResource> GetDXResource() { return m_Resource; }

	private:

		Ref<DirectX12ConstantBufferResource> m_Resource;
	};



	class DirectX12RWConstantBuffer : public RWConstantBuffer
	{
	public:
		DirectX12RWConstantBuffer(uint32 size);
		DirectX12RWConstantBuffer(Ref<ConstantBufferResource> resource);

		virtual void SetData(const void* data) override;

		virtual uint32 GetDescriptorLocation() override { return m_Resource->m_UAVHandle.GetIndex(); }

		virtual Ref<ConstantBufferResource> GetResource() override { return m_Resource; }

		Ref<DirectX12ConstantBufferResource> GetDXResource() { return m_Resource; }

	private:

		Ref<DirectX12ConstantBufferResource> m_Resource;
	};
}
