#include "pch.h"
#include "CommandList.h"
#include "Renderer.h"

#include "Engine/Util/Performance.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/DirectX12CommandList.h"
#endif

namespace Engine
{
	Ref<CommandList> Engine::CommandList::Create(CommandListType type)
	{
		CREATE_PROFILE_FUNCTIONI();
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			return CreateRef<DirectX12CommandList>(type);
		default:
			break;
		}
		return Ref<CommandList>();
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer)
	{
		for (uint32 i = 0; i < frameBuffer->GetAttachments().size(); i++)
			ClearRenderTarget(frameBuffer, i);
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment)
	{
		//Math::Vector4 color = frameBuffer->GetAttachment(attachment).clearColor;
		ClearRenderTarget(frameBuffer, attachment, { 0,0,0,0 });
	}

	void CommandList::ClearRenderTarget(Ref<FrameBuffer> frameBuffer, uint32 attachment, const Math::Vector4& color)
	{
		ClearRenderTarget(frameBuffer->GetAttachment(attachment), color);
	}

	void CommandList::ClearRenderTarget(Ref<RenderTarget2D> renderTarget)
	{
		const static Math::Vector4 color = { 0,0,0,0 };
		ClearRenderTarget(renderTarget, color);
	}

}
