#include "pch.h"
#include "FrameBuffer.h"

#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	FrameBuffer::FrameBuffer(const std::vector<Ref<RenderTarget2D>>& attachments)
	{
		m_Attachments = attachments;
	}

	void FrameBuffer::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < m_Attachments.size(); i++)
			m_Attachments[i]->Resize(width, height);
	}

	std::vector<TextureFormat> FrameBuffer::GetSpecification()
	{
		std::vector<TextureFormat> formates;
		TextureFormat depthFormat = TextureFormat::None;
		for (Ref<RenderTarget2D> rt : m_Attachments)
		{
			TextureFormat format = rt->GetResource()->GetFormat();
			if (IsDepthStencil(format))
				depthFormat = format;
			else
				formates.push_back(format);
		}
		formates.push_back(depthFormat);
		return formates;
	}

	bool FrameBuffer::HasDepthAttachment() const
	{
		return IsDepthStencil(m_Attachments.back()->GetResource()->GetFormat());
	}

	Ref<FrameBuffer> FrameBuffer::Create(const std::vector<Ref<RenderTarget2D>>& attachments)
	{
		return CreateRef<FrameBuffer>(attachments);
	}
}
