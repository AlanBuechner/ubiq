#pragma once
#include "Engine/Renderer/Abstractions/Resources/InstanceBuffer.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	class UploadBufferResource;
}

namespace Engine
{
	class DirectX12InstanceBufferResource : public InstanceBufferResource
	{
	public:
		DirectX12InstanceBufferResource(uint32 capacity, uint32 stride);
		virtual ~DirectX12InstanceBufferResource() override;

		ID3D12Resource* GetBuffer() { return m_Buffer; }

		virtual void SetData(const void* data, uint32 count) override;

	protected:
		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetGPUState(ResourceState state) override;

	private:
		ID3D12Resource* m_Buffer;
		UploadBufferResource* m_UploadBuffer;
	};

	class DirectX12InstanceBufferView : public InstanceBufferView
	{
	public:
		virtual ~DirectX12InstanceBufferView() override = default;

		virtual void Bind(InstanceBufferResource* resource) override;

		D3D12_VERTEX_BUFFER_VIEW GetView() { return m_View; }

	private:
		D3D12_VERTEX_BUFFER_VIEW m_View;

	};
}
