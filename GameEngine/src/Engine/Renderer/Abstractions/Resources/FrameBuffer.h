#pragma once

#include "Engine/Core/Core.h"
#include "Texture.h"
#include "ResourceState.h"

namespace Engine
{
	struct FrameBufferDescription
	{
		Utils::Vector<TextureFormat> formats;
		MSAASampleCount sampleCount;

		bool operator ==(const FrameBufferDescription& other) const
		{
			return formats == other.formats && sampleCount == other.sampleCount;
		}
	};

	class FrameBuffer
	{
	public:
		FrameBuffer(const Utils::Vector<Ref<RenderTarget2D>>& attachments);

		void Resize(uint32 width, uint32 height);

		Ref<RenderTarget2D> GetAttachment(uint32 i) const { return m_Attachments[i]; }
		Ref<RenderTarget2D> GetDepthAttachment() const { return HasDepthAttachment() ? m_Attachments.Back() : nullptr; }
		const Utils::Vector<Ref<RenderTarget2D>>& GetAttachments() const { return m_Attachments; }

		Utils::Vector<TextureFormat> GetFormats();
		FrameBufferDescription GetDescription();

		bool HasDepthAttachment() const;

		static Ref<FrameBuffer> Create(const Utils::Vector<Ref<RenderTarget2D>>& spec);

	private:
		void ValidateSampleCount();


	private:

		Utils::Vector<Ref<RenderTarget2D>> m_Attachments;

	};
}


struct FBDHash {
	std::size_t operator()(const Engine::FrameBufferDescription& desc) const {
		std::hash<uint32> hasher;
		size_t seed = 0;
		for (Engine::TextureFormat f : desc.formats) {
			seed ^= hasher((uint32)f) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		}
		seed ^= hasher((uint32)desc.sampleCount) + 0x9e3779b9 + (seed << 6) + (seed >> 2);
		return seed;
	}
};
