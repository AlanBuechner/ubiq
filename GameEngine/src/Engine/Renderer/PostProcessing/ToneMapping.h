#pragma once
#include "PostProcess.h"
#include "Engine/Renderer/Shader.h"

namespace Engine
{
	class ToneMapping : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh) override;

	private:
		Ref<Shader> m_ToneMappingShader;

	};
}
