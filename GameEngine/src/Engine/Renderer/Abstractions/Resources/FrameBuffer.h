#pragma once

#include "Engine/Core/Core.h"
#include "Texture.h"
#include "ResourceState.h"

namespace Engine
{
	class FrameBuffer
	{
	public:
		FrameBuffer(const std::vector<Ref<RenderTarget2D>>& attachments);

		void Resize(uint32 width, uint32 height);

		Ref<RenderTarget2D> GetAttachment(uint32 i) const { return m_Attachments[i]; }
		Ref<RenderTarget2D> GetDepthAttachment() const { return HasDepthAttachment() ? m_Attachments.back() : nullptr; }
		const std::vector<Ref<RenderTarget2D>>& GetAttachments() const { return m_Attachments; }

		bool HasDepthAttachment() const;

		static Ref<FrameBuffer> Create(const std::vector<Ref<RenderTarget2D>>& spec);

	private:

		std::vector<Ref<RenderTarget2D>> m_Attachments;

	};
}
