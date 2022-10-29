#pragma once

#include "Engine/Core/Core.h"

namespace Engine
{
	enum class FrameBufferTextureFormat
	{
		None = 0,

		// Color
		RGBA8,
		RED_INTEGER,

		// Depth/Stencil
		DEPTH24STENCIL8,

		Depth = DEPTH24STENCIL8
	};

	struct FrameBufferTextureSpecification
	{
		FrameBufferTextureSpecification() = default;
		FrameBufferTextureSpecification(FrameBufferTextureFormat format, Math::Vector4 color) :
			TextureFormat(format), ClearColor(color)
		{}

		FrameBufferTextureFormat TextureFormat = FrameBufferTextureFormat::None;
		Math::Vector4 ClearColor;
		inline bool IsDepthStencil() const
		{
			switch (TextureFormat)
			{
			case Engine::FrameBufferTextureFormat::DEPTH24STENCIL8:
				return true;
			}
			return false;
		}

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

		bool SwapChainTarget = false;
	};

	class FrameBuffer
	{
	public:

		enum State
		{
			RenderTarget,
			SRV,
			Common,
		};

		virtual ~FrameBuffer() = default;

		virtual void Resize(uint32 width, uint32 height) = 0;

		virtual uint64 GetAttachmentRenderHandle(uint32 index = 0) const = 0;
		virtual uint64 GetAttachmentShaderHandle(uint32 index = 0) const = 0;
		virtual int ReadPixle(uint32 attachment, int x, int y) = 0;

		virtual const FrameBufferSpecification& GetSpecification() const = 0;
		virtual bool HasDepthAttachment() const = 0;

		virtual State GetState() = 0;
		virtual void SetState(State state) = 0;
		virtual bool Cleared() = 0;
		virtual void ResetClear() = 0; // only called by render graph

		static Ref<FrameBuffer> Create(const FrameBufferSpecification& spec);
	};
}
