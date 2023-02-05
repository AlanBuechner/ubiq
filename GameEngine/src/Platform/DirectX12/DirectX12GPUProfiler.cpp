#include "pch.h"
#include "DirectX12GPUProfiler.h"
#include "DX.h"
#include "WinPixEventRuntime/pix3.h"

namespace Engine
{

	void DirectX12GPUProfiler::BeginEvent(Ref<DirectX12CommandList> commandList, const std::string& eventName)
	{
		PIXBeginEvent(commandList->GetCommandList().Get(), 0, eventName.c_str());
	}

	void DirectX12GPUProfiler::EndEvent(Ref<DirectX12CommandList> commandList)
	{
		PIXEndEvent(commandList->GetCommandList().Get());
	}

}
