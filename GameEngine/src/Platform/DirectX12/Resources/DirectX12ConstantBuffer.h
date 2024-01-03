#pragma once
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"
#include "DirectX12ResourceManager.h"
#include "DirectX12Descriptors.h"
#include "Platform/DirectX12/DX.h"


namespace Engine
{
	class DirectX12ConstantBufferResource : public ConstantBufferResource
	{
	public:
		DirectX12ConstantBufferResource(uint32 size);
		virtual ~DirectX12ConstantBufferResource() override;

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }

		virtual void SetData(const void* data) override;

	private:

		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetState(ResourceState state) override;

	private:
		ID3D12Resource* m_Buffer;

	};

	class DirectX12ConstantBufferCBVDescriptorHandle : public ConstantBufferCBVDescriptorHandle
	{
	public:
		DirectX12ConstantBufferCBVDescriptorHandle();
		virtual ~DirectX12ConstantBufferCBVDescriptorHandle() override { m_CBVHandle.Free(); }

		virtual uint64 GetGPUHandlePointer() const override { return m_CBVHandle.gpu.ptr; }
		virtual uint32 GetIndex() const override { return m_CBVHandle.GetIndex(); }
		virtual void Bind(ConstantBufferResource* resource) override;

		const DirectX12DescriptorHandle& GetHandle() { return m_CBVHandle; }

	private:
		DirectX12DescriptorHandle m_CBVHandle;
	};
}
