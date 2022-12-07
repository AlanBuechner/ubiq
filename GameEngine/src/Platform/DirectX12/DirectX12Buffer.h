#pragma once
#include "Engine/Renderer/Buffer.h"
#include "DX.h"

namespace Engine
{
	class ENGINE_API DirectX12VertexBuffer : public VertexBuffer
	{
	public:
		DirectX12VertexBuffer(uint32 count, uint32 stride);
		DirectX12VertexBuffer(const void* vertices, uint32 count, uint32 stride);
		virtual ~DirectX12VertexBuffer();

		virtual void SetLayout(const BufferLayout& layout) override { m_Layout = layout; };
		virtual BufferLayout& GetLayout() override { return m_Layout; };

		virtual void SetData(const void* data, uint32 count) override;

		D3D12_VERTEX_BUFFER_VIEW GetView() { return m_View; }

	private:
		void CreateBuffer(uint32 count);

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		D3D12_VERTEX_BUFFER_VIEW m_View;

		BufferLayout m_Layout;
		uint32 m_Count = 0;
		uint32 m_Stride = 0;
	};

	class ENGINE_API DirectX12IndexBuffer : public IndexBuffer
	{
	public:
		DirectX12IndexBuffer(uint32 count);
		DirectX12IndexBuffer(const uint32* indices, uint32 count);
		virtual ~DirectX12IndexBuffer();

		virtual uint32 GetCount() override { return m_Count; }

		virtual void SetData(const uint32* data, uint32 count) override;

		const D3D12_INDEX_BUFFER_VIEW& GetView() { return m_View; }

	private:
		void CreateBuffer(uint32 count);

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		D3D12_INDEX_BUFFER_VIEW m_View;

		uint32 m_Count;
	};
}
