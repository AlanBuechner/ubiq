#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class CommandList;
	class CPUCommandList;
}

namespace Engine
{
	class GPUTimer
	{
	public:
		static void BeginEvent(CommandList* commandList, const std::string& eventName);
		static void EndEvent(CommandList* commandList);
		static inline void BeginEvent(Ref<CommandList> commandList, const std::string& eventName) { BeginEvent(commandList.get(), eventName); }
		static inline void EndEvent(Ref<CommandList> commandList) { EndEvent(commandList.get()); }

		static void BeginEvent(Ref<CPUCommandList> commandList, const char* eventName); // static named event
		static void BeginEvent(Ref<CPUCommandList> commandList, const std::string& eventName); // dynamically named event
		static void EndEvent(Ref<CPUCommandList> commandList);

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
