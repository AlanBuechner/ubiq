#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/CommandList.h"
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph/RenderGraph.h"

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
		virtual void Init(const PostProcessInput& input, SceneData& scene) = 0;
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh) = 0;

	protected:
		SceneData* m_Scene;
	};
}
