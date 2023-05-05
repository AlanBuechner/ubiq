#pragma once
#include "Engine/Renderer/Resources/FrameBuffer.h"
#include "DirectX12Descriptors.h"
#include "DX.h"

namespace Engine
{
	DXGI_FORMAT UbiqToDXGI(FrameBufferTextureFormat format);

	class DirectX12FrameBuffer : public FrameBuffer
	{
	public:
		DirectX12FrameBuffer(const FrameBufferSpecification& spec);
		virtual ~DirectX12FrameBuffer();

		void Invalidate();

		virtual void Resize(uint32 width, uint32 height) override;

		virtual uint64 GetAttachmentRenderHandle(uint32 index) const override;
		virtual uint64 GetAttachmentShaderHandle(uint32 index) const override;
		virtual uint32 GetAttachmentShaderDescriptoLocation(uint32 index) const override;
		virtual int ReadPixle(uint32 index, int x, int y) override;

		virtual const FrameBufferSpecification& GetSpecification() const override { return m_Spec; }
		virtual bool HasDepthAttachment() const override { return m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None; };

		wrl::ComPtr<ID3D12Resource>& GetBuffer(uint32 i) { return m_Buffers[i]; }
		void SetDescriptorHandle(uint32 i, DirectX12DescriptorHandle handle) { CORE_ASSERT(i < m_TargetHandles.size(), ""); m_TargetHandles[i] = handle; }

		virtual bool Cleared() override { return m_Cleared; }
		virtual void ResetClear() override { m_Cleared = false; }

		static D3D12_RESOURCE_STATES GetDXState(FrameBufferState state);
		static D3D12_RESOURCE_STATES GetDXDepthState(FrameBufferState state);

		void Clear() { m_Cleared = true; } // only gets called by command list


	private:
		void CreateAttachment(uint32 i);

	private:
		bool m_Cleared = false;

		std::vector<wrl::ComPtr<ID3D12Resource>> m_Buffers;
		FrameBufferSpecification m_Spec;

		std::vector<FrameBufferTextureSpecification> m_AttachmentSpecs;
		FrameBufferTextureSpecification m_DepthAttachmentSpec = { FrameBufferTextureFormat::None, {0,0,0,0} };

		std::vector<DirectX12DescriptorHandle>  m_TargetHandles;
		std::vector<DirectX12DescriptorHandle>  m_SRVHandles;
	};
}
