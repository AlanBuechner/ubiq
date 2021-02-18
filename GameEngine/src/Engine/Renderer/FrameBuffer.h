#pragma once

#include "Engine/Core/core.h"

namespace Engine
{
	enum class FramBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,

		// Depth/Stencil
		DEPTH24STENCIL8,

		Depth = DEPTH24STENCIL8
	};

	struct FramBufferTextureSpecification
	{
		FramBufferTextureSpecification() = default;
		FramBufferTextureSpecification(FramBufferTextureFormat format) :
			TextureFormat(format) 
		{}

		FramBufferTextureFormat TextureFormat = FramBufferTextureFormat::None;

	};

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<FramBufferTextureSpecification> attachments) :
			Attachments(attachments)
		{}

		std::vector<FramBufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification
	{
		uint32_t Width, Height;
		FrameBufferAttachmentSpecification Attachments;
		uint32_t Samples = 1;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:

		virtual ~FrameBuffer() = default;

		virtual void Bind() = 0;
		virtual void Unbind() = 0;

		virtual void Resize(uint32_t width, uint32_t height) = 0;

		virtual uint32_t GetColorAttachmentRendererID(uint32_t index = 0) const = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}