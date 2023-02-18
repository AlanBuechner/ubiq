#pragma once
#include "Engine/Core/Core.h"
#include "CommandList.h"

namespace Engine
{
	class GPUTimer
	{
	public:
		static void BeginEvent(Ref<CommandList> commandList, const std::string& eventName);
		static void EndEvent(Ref<CommandList> commandList);
	};

	class GPUProfiler
	{
	public:
		static void Init();
		static void SetTragetWindow(void* nativeWindow);
		static void TriggerGPUCapture();

		static void StartFrame();
		static void EndFrame();
	};
}
