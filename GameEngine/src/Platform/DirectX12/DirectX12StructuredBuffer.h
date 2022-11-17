#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/StructuredBuffer.h"
#include "DirectX12ResourceManager.h"
#include "DX.h"

namespace Engine
{
	class DirectX12StructuredBuffer : public StructuredBuffer
	{
	public:
		DirectX12StructuredBuffer(uint32 stride, uint32 count);
		~DirectX12StructuredBuffer();

		virtual void Resize(uint32 count) override;

		virtual void SetData(void* data, uint32 count = 1, uint32 start = 0) override;

		virtual uint32 GetDescriptorLocation() const override;

	private:
		wrl::ComPtr<ID3D12Resource> m_Buffer;
		DirectX12DescriptorHandle m_SRVhandle;

		uint32 m_Stride;
		uint32 m_Count;

	};
}
