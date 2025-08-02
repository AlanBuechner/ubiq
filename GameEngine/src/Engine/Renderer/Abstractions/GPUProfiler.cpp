#include "pch.h"
#include "GPUProfiler.h"
#include "Engine/Renderer/Renderer.h"
#include "CommandList.h"
#include <memory>

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "Platform/DirectX12/DirectX12GPUProfiler.h"
#endif

namespace Engine
{

	void GPUTimer::BeginEvent(CommandList* commandList, const std::string& eventName)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::BeginEvent((DirectX12CommandList*)commandList, eventName);
			break;
		default:
			break;
		}
	}

	void GPUTimer::EndEvent(CommandList* commandList)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::EndEvent((DirectX12CommandList*)commandList);
			break;
		default:
			break;
		}
	}


	void GPUTimer::BeginEvent(Ref<CPUCommandList> commandList, const char* eventName)
	{
		commandList->BeginEvent(eventName);
	}

	void GPUTimer::BeginEvent(Ref<CPUCommandList> commandList, const std::string& eventName)
	{
		commandList->BeginEvent(eventName);
	}

	void GPUTimer::EndEvent(Ref<CPUCommandList> commandList)
	{
		commandList->EndEvent();
	}




	void GPUProfiler::Init()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::Init();
			break;
		default:
			break;
		}
	}

	void GPUProfiler::SetTragetWindow(void* nativeWindow)
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::SetTargetWindow(nativeWindow);
			break;
		default:
			break;
		}
	}

	void GPUProfiler::TriggerGPUCapture()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::TriggerPixGPUCapture();
			break;
		default:
			break;
		}
	}

	void GPUProfiler::StartFrame()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::BeginFrame();
			break;
		default:
			break;
		}
	}

	void GPUProfiler::EndFrame()
	{
		switch (Renderer::GetAPI())
		{
		case RendererAPI::DirectX12:
			DirectX12GPUProfiler::EndFrame();
			break;
		default:
			break;
		}
	}

}

