#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Renderer/RenderGraph/RenderGraph.h"

#include <unordered_map>
#include <string.h>

namespace Game
{
	struct PostProcessInput
	{
		std::unordered_map<std::string, Engine::Ref<Engine::Texture2D>> m_TextureHandles;
		std::unordered_map<std::string, Engine::Ref<Engine::ConstantBuffer>> m_CBHandles;
	};

	class PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) = 0;
		virtual void RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh) = 0;

		virtual void OnViewportResize(uint32 width, uint32 height) {}

	protected:
		SceneData* m_Scene;
	};
}
