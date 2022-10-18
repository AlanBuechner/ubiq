#pragma once
#include "Engine/Renderer/ConstantBuffer.h"
#include "DirectX12Descriptors.h"
#include "DX.h"


namespace Engine
{
	class DirectX12ConstantBuffer : public ConstantBuffer
	{
	public:
		DirectX12ConstantBuffer(uint32 size);
		~DirectX12ConstantBuffer();

		virtual void SetData(void* data) override;

		wrl::ComPtr<ID3D12Resource> GetBuffer() { return m_Buffer; }
		DirectX12DescriptorHandle GetHandle() { return m_Handle; }

	private:
		uint32 m_Size;

		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_Handle;
	};
}
