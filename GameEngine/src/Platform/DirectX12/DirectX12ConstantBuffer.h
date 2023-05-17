#pragma once
#include "Engine/Renderer/Resources/ConstantBuffer.h"
#include "DirectX12Descriptors.h"
#include "DX.h"


namespace Engine
{
	class DirectX12ConstantBufferResource : public ConstantBufferResource
	{
	public:
		DirectX12ConstantBufferResource(uint32 size);
		~DirectX12ConstantBufferResource();

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }

		const DirectX12DescriptorHandle& GetCBVHandle() { return m_CBVHandle; }
		const DirectX12DescriptorHandle& GetUAVHandle() { return m_UAVHandle; }


	private:
		void SetData(const void* data);

		void CreateCBVHandle();
		void CreateUAVHandle();

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_CBVHandle;
		DirectX12DescriptorHandle m_UAVHandle;

		friend class DirectX12ConstantBuffer;
		friend class DirectX12RWConstantBuffer;
	};


	class DirectX12ConstantBuffer : public ConstantBuffer
	{
	public:
		DirectX12ConstantBuffer(uint32 size);
		DirectX12ConstantBuffer(Ref<ConstantBufferResource> resource);

		virtual void SetData(const void* data) override { m_Resource->SetData(data); }

		virtual uint32 GetDescriptorLocation() override { return m_Resource->m_CBVHandle.GetIndex(); }

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

		virtual void SetData(const void* data) override { m_Resource->SetData(data); }

		virtual uint32 GetDescriptorLocation() override { return m_Resource->m_UAVHandle.GetIndex(); }

		virtual Ref<ConstantBufferResource> GetResource() override { return m_Resource; }

		Ref<DirectX12ConstantBufferResource> GetDXResource() { return m_Resource; }

	private:

		Ref<DirectX12ConstantBufferResource> m_Resource;
	};
}
