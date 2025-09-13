#include "pch.h"
#include "FrameBuffer.h"

#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	FrameBuffer::FrameBuffer(const Utils::Vector<Ref<RenderTarget2D>>& attachments)
	{
		m_Attachments = attachments;
		ValidateSampleCount();
	}

	void FrameBuffer::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < m_Attachments.Count(	); i++)
			m_Attachments[i]->Resize(width, height);
	}

	Utils::Vector<TextureFormat> FrameBuffer::GetFormats()
	{
		Utils::Vector<TextureFormat> formats;
		for (Ref<RenderTarget2D> rt : m_Attachments)
			formats.Push(rt->GetResource()->GetFormat());
		return formats;
	}

	FrameBufferDescription FrameBuffer::GetDescription()
	{
		ValidateSampleCount();

		FrameBufferDescription desc;
		desc.formats = GetFormats();
		desc.sampleCount = m_Attachments[0]->GetSampleCount();
		return desc;
	}

	bool FrameBuffer::HasDepthAttachment() const
	{
		if (m_Attachments.Empty()) return false;
		return IsTextureFormatDepthStencil(m_Attachments.Back()->GetResource()->GetFormat());
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Utils::Vector<Ref<RenderTarget2D>>& attachments)
	{
		return CreateRef<FrameBuffer>(attachments);
	}

	void FrameBuffer::ValidateSampleCount()
	{
		MSAASampleCount sampleCount = m_Attachments[0]->GetSampleCount();
		for (uint32 i = 0; i < m_Attachments.Count(); i++)
			CORE_ASSERT(m_Attachments[i]->GetSampleCount() == sampleCount, "All render targes in frame buffer need the same MSAA sample count", "")
	}

}
