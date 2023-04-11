#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/FrameBuffer.h"

namespace Engine
{
	class PostProcess
	{
	public:
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget) = 0;
	};
}
