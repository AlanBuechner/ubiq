#pragma once

#include "Engine/Core/Core.h"
#include "Engine/Renderer/Abstractions/CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/FrameBuffer.h"

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
	protected:
		PostProcess() = default;

	public:

		void Initialize(Engine::Ref<Engine::CPUCommandList> commandList, PostProcessInput* input, Engine::Ref<Engine::Mesh> screenMesh);
		virtual void Init() = 0;
		virtual void RecordCommands(Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src) = 0;

		virtual void OnViewportResize(uint32 width, uint32 height) {}

	protected:
		PostProcessInput* m_Input;
		Engine::Ref<Engine::Mesh> m_ScreenMesh;
		Engine::Ref<Engine::CPUCommandList> m_CommandList;
	};

}
