#pragma once
#include "Engine/Renderer/FrameBuffer.h"
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
		virtual int ReadPixle(uint32 index, int x, int y) override;

		virtual const FrameBufferSpecification& GetSpecification() const { return m_Spec; }
		virtual bool HasDepthAttachment() const { return m_DepthAttachmentSpec.TextureFormat != FrameBufferTextureFormat::None; };

		wrl::ComPtr<ID3D12Resource>& GetBuffer(uint32 i) { return m_Buffers[i]; }
		void SetDescriptorHandle(uint32 i, DirectX12DescriptorHandle handle) { CORE_ASSERT(i < m_TargetHandles.size(), ""); m_TargetHandles[i] = handle; }

	private:
		void CreateAttachment(uint32 i);

	private:
		std::vector<wrl::ComPtr<ID3D12Resource>> m_Buffers;
		FrameBufferSpecification m_Spec;

		std::vector<FrameBufferTextureSpecification> m_AttachmentSpecs;
		FrameBufferTextureSpecification m_DepthAttachmentSpec = { FrameBufferTextureFormat::None, {0,0,0,0} };

		std::vector<DirectX12DescriptorHandle>  m_TargetHandles;
		std::vector<DirectX12DescriptorHandle>  m_SRVHandles;
		uint32 m_DepthAttachment;
	};
}