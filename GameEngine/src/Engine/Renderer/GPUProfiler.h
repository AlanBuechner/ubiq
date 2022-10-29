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
}
