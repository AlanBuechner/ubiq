#pragma once
#include "PostProcess.h"
#include "Engine/Renderer/Shaders/Shader.h"

namespace Game
{
	class ToneMapping : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh) override;

	private:
		Engine::Ref<Engine::Shader> m_ToneMappingShader;

	};
}
