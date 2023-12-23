#include "pch.h"
#include "Commands.h"
#include "Engine/Renderer/Renderer.h"

#ifdef PLATFORM_WINDOWS
#include "Platform/DirectX12/Commands/DirectX12Commands.h"
#endif

namespace Engine
{
	void (*TransitionCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRenderTargetCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*ClearRenderTargetCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetShaderCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRootConstantCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetConstantBufferCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetStructuredBufferCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetTextureCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*SetRWTextureCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*DrawMeshCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
	void (*DisbatchComputeCommand::RecoardCommandFunc)(CommandList&, Command&) = nullptr;
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

