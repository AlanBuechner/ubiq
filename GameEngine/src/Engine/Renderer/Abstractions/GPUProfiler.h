#pragma once
#include "Engine/Core/Core.h"
#include "Utils/Performance.h"

namespace tracy
{
	struct SourceLocationData;
}

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


#define BEGIN_EVENT_TRACE_GPU(cmdList, name) CREATE_SOURCE_LOC(name); cmdList->BeginGPUEvent(&SOURCE_LOC_NAME);
#define END_EVENT_TRACE_GPU(cmdList) cmdList->EndGPUEvent();
