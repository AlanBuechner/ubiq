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

		virtual void Resize(uint32_t width, uint32_t height) override;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index) const override { CORE_ASSERT(index < m_ColorAttachments.size(), ""); return m_ColorAttachments[index]; }

		virtual const FrameBufferSpecification& GetSpecification() const { return m_Spec; }

	private:
		uint32_t m_RendererID = 0;
		FrameBufferSpecification m_Spec;

		std::vector<FramBufferTextureSpecification> m_ColorAttachmentSpecs;
		FramBufferTextureSpecification m_DepthAttachmentSpec = FramBufferTextureFormat::None;

		std::vector<uint32_t>  m_ColorAttachments;
		uint32_t m_DepthAttachment;
	};
}