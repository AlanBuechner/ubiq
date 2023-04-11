#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/FrameBuffer.h"

#include <unordered_map>
#include <string.h>

namespace Engine
{
	struct PostProcessInput
	{
		std::unordered_map<std::string, uint64> m_TextureHandles;
	};

	class PostProcess
	{
	public:
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, const PostProcessInput& input) = 0;
	};
}
