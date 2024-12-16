#pragma once
#include "Engine/Renderer/Abstractions/Resources/Buffer.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	// VertexBuffer -------------------------------------------------------------------------------------

	class DirectX12VertexBufferView : public VertexBufferView
	{
	public:
		virtual ~DirectX12VertexBufferView() override = default;

		virtual void Bind(StructuredBufferResource* resource) override;

		D3D12_VERTEX_BUFFER_VIEW& GetView() { return m_View; }

	private:
		D3D12_VERTEX_BUFFER_VIEW m_View;
	};

	// IndexBuffer -------------------------------------------------------------------------------------

	class DirectX12IndexBufferView : public IndexBufferView
	{
	public:
		virtual ~DirectX12IndexBufferView() override = default;

		virtual void Bind(StructuredBufferResource* resource) override;

		D3D12_INDEX_BUFFER_VIEW& GetView() { return m_View; }

	private:
		D3D12_INDEX_BUFFER_VIEW m_View;
	};
}
