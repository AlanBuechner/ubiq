#include "pch.h"
#include "GPUProfiler.h"
#include "Renderer.h"
#include <memory>

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "Platform/DirectX12/DirectX12GPUProfiler.h"
#endif

namespace Engine
{

	void GPUTimer::BeginEvent(Ref<CommandList> commandList, const std::string& eventName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::BeginEvent(std::dynamic_pointer_cast<DirectX12CommandList>(commandList), eventName);
			break;
		default:
			break;
		}
	}

	void GPUTimer::EndEvent(Ref<CommandList> commandList)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::EndEvent(std::dynamic_pointer_cast<DirectX12CommandList>(commandList));
			break;
		default:
			break;
		}
	}

}

