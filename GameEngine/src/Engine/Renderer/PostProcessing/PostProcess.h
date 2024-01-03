#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph/RenderGraph.h"

#include <unordered_map>
#include <string.h>

namespace Engine
{
	struct PostProcessInput
	{
		std::unordered_map<std::string, Ref<Texture2D>> m_TextureHandles;
		std::unordered_map<std::string, Ref<ConstantBuffer>> m_CBHandles;
	};

	class PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) = 0;
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh) = 0;

		virtual void OnViewportResize(uint32 width, uint32 height) {}

	protected:
		SceneData* m_Scene;
	};
}
