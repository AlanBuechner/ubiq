#pragma once
#include "Engine/Renderer/FrameBuffer.h"

namespace Engine
{
	class OpenGLFrameBuffer : public FrameBuffer
	{
	public:
		OpenGLFrameBuffer(const FrameBufferSpecification& spec);
		virtual ~OpenGLFrameBuffer();

		void Invalidate();

		virtual void Bind() override;
		virtual void Unbind() override;

		virtual void Resize(uint32 width, uint32 height) override;

		virtual uint32 GetColorAttachmentRendererID(uint32 index) const override { CORE_ASSERT(index < m_ColorAttachments.size(), ""); return m_ColorAttachments[index]; }
		virtual int ReadPixle(uint32 index, int x, int y) override;

		virtual void ClearAttachment(uint32 attachmentIndex, int value) override;

		virtual const FrameBufferSpecification& GetSpecification() const { return m_Spec; }

	private:
		uint32 m_RendererID = 0;
		FrameBufferSpecification m_Spec;

		std::vector<FrameBufferTextureSpecification> m_ColorAttachmentSpecs;
		FrameBufferTextureSpecification m_DepthAttachmentSpec = FrameBufferTextureFormat::None;

		std::vector<uint32>  m_ColorAttachments;
		uint32 m_DepthAttachment;
	};
}
