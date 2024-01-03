#pragma once
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	class DirectX12VertexBufferResource : public VertexBufferResource
	{
	public:
		DirectX12VertexBufferResource(uint32 count, uint32 stride);
		virtual ~DirectX12VertexBufferResource();

		ID3D12Resource* GetBuffer() { return m_Buffer; }

		virtual void SetData(const void* data) override;

	protected:
		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetState(ResourceState state) override;

	private:
		ID3D12Resource* m_Buffer;
	};


	class DirectX12VertexBufferView : public VertexBufferView
	{
	public:
		virtual ~DirectX12VertexBufferView() override = default;

		virtual void Bind(VertexBufferResource* resource) override;

		D3D12_VERTEX_BUFFER_VIEW& GetView() { return m_View; }

	private:
		D3D12_VERTEX_BUFFER_VIEW m_View;
	};

	// IndexBuffer -------------------------------------------------------------------------------------

	class DirectX12IndexBufferResource : public IndexBufferResource
	{
	public:
		DirectX12IndexBufferResource(uint32 count);
		virtual ~DirectX12IndexBufferResource();

		ID3D12Resource* GetBuffer() { return m_Buffer; }

		virtual void SetData(const void* data) override;

	protected:
		virtual void* GetGPUResourcePointer() override { return m_Buffer; }
		virtual uint32 GetState(ResourceState state) override;

	private:
		ID3D12Resource* m_Buffer;
	};


	class DirectX12IndexBufferView : public IndexBufferView
	{
	public:
		virtual ~DirectX12IndexBufferView() override = default;

		virtual void Bind(IndexBufferResource* resource) override;

		D3D12_INDEX_BUFFER_VIEW& GetView() { return m_View; }

	private:
		D3D12_INDEX_BUFFER_VIEW m_View;
	};
}
