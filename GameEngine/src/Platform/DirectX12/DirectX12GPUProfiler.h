#pragma once
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include "DirectX12CommandList.h"

namespace Engine
{
	class DirectX12GPUProfiler
	{
	public:
		static void Init();

		static void BeginEvent(DirectX12CommandList* commandList, const std::string& eventName);
		static void EndEvent(DirectX12CommandList* commandList);

		static void SetTargetWindow(void* nativeWindow);

		static void TriggerPixGPUCapture();

		static void BeginFrame();
		static void EndFrame();

	private:
		static bool s_CaptureFrame;

	};

}
