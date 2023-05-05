#pragma once
#include "Engine/Renderer/Resources/InstanceBuffer.h"
#include "DX.h"

namespace Engine
{
	class DirectX12InstanceBuffer : public InstanceBuffer
	{
	public:
		DirectX12InstanceBuffer(uint32 stride, uint32 count);
		~DirectX12InstanceBuffer();

		virtual void SetData(uint32 start, uint32 count, const void* data) override;
		virtual void PushBack(uint32 count, const void* data) override;
		virtual void PopBack() override { m_UploadCount--; }
		virtual void Clear() override { m_UploadCount = 0; }
		virtual void Apply() override;
		virtual void* At(uint32 index) override;
		virtual uint32 GetCount() override { return m_UploadCount; }

		D3D12_VERTEX_BUFFER_VIEW GetView() { return m_View; }

	private:
		void CreateBuffer(uint32 count);
		void CreateUploadBuffer(uint32 count);

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		wrl::ComPtr<ID3D12Resource> m_UploadBuffer;
		D3D12_VERTEX_BUFFER_VIEW m_View;

		void* m_MapLoc = nullptr;

		uint32 m_Count = 0;
		uint32 m_Stride = 0;

		uint32 m_Capacity = 0;
		uint32 m_UploadCount = 0;
	};
}
