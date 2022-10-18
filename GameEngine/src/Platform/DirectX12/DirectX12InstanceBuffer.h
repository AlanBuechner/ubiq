#pragma once
#include "Engine/Renderer/InstanceBuffer.h"
#include "DX.h"

namespace Engine
{
	class DirectX12InstanceBuffer : public InstanceBuffer
	{
	public:
		DirectX12InstanceBuffer(uint32 stride, uint32 count);
		~DirectX12InstanceBuffer();

		virtual void SetData(uint32 start, uint32 count, const void* data) override;
		virtual uint32 GetCount() override { return m_Count; }

		D3D12_VERTEX_BUFFER_VIEW GetView() { return m_View; }


	private:
		void CreateBuffer();

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		D3D12_VERTEX_BUFFER_VIEW m_View;

		uint32 m_Count = 0;
		uint32 m_Stride = 0;
	};
}
