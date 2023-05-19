#pragma once

#include "Engine/Core/Core.h"
#include "Texture.h"
#include "ResourceState.h"

namespace Engine
{

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(TextureFormat format, Math::Vector4 color) :
			textureFormat(format), clearColor(color)
		{}

		TextureFormat textureFormat = TextureFormat::None;
		Math::Vector4 clearColor;

	};

	struct FrameBufferAttachmentSpecification
	{
		FrameBufferAttachmentSpecification() = default;
		FrameBufferAttachmentSpecification(std::initializer_list<FrameBufferTextureSpecification> attachments) :
			Attachments(attachments)
		{}

		std::vector<FrameBufferTextureSpecification> Attachments;
	};

	struct FrameBufferSpecification
	{
		uint32 Width, Height;
		FrameBufferAttachmentSpecification Attachments;
		uint32 Samples = 1;
		ResourceState InitalState = ResourceState::Common;

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:
		virtual ~FrameBuffer() = default;

		virtual void Resize(uint32 width, uint32 height) = 0;

		virtual uint64 GetAttachmentRenderHandle(uint32 index = 0) const = 0;
		virtual uint64 GetAttachmentShaderHandle(uint32 index = 0) const = 0;
		virtual uint32 GetAttachmentShaderDescriptoLocation(uint32 index = 0) const = 0;
		virtual int ReadPixle(uint32 attachment, int x, int y) = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;
		virtual bool HasDepthAttachment() const = 0;

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}
