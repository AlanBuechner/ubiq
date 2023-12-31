#include "pch.h"
#include "Commands.h"
#include "Engine/Renderer/Renderer.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/Commands/DirectX12Commands.h"
#endif

namespace Engine
{
	void (*TransitionCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRenderTargetCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*ClearRenderTargetCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetShaderCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRootConstantCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetConstantBufferCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetStructuredBufferCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetTextureCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRWTextureCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*DrawMeshCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
	void (*DisbatchComputeCommand::RecordCommandFunc)(CommandList&, Command&) = nullptr;
}

namespace Engine
{

	void Command::InitCommands()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			InitDirectX12Commands();
		default:
			break;
		}
	}

}

