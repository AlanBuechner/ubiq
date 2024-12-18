#include "pch.h"
#include "FrameBuffer.h"

#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	FrameBuffer::FrameBuffer(const Utils::Vector<Ref<RenderTarget2D>>& attachments)
	{
		m_Attachments = attachments;
	}

	void FrameBuffer::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < m_Attachments.Count(	); i++)
			m_Attachments[i]->Resize(width, height);
	}

	bool FrameBuffer::HasDepthAttachment() const
	{
		return IsTextureFormatDepthStencil(m_Attachments.Back()->GetResource()->GetFormat());
	}

	Ref<FrameBuffer> FrameBuffer::Create(const Utils::Vector<Ref<RenderTarget2D>>& attachments)
	{
		return CreateRef<FrameBuffer>(attachments);
	}
}
