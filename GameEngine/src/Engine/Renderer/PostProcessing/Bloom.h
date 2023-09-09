#pragma once
#include "Engine/Core/Core.h"
#include "PostProcess.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Resources/ConstantBuffer.h"

namespace Engine
{
	class Bloom : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Ref<Shader> m_BloomShader;
		uint32 m_NumberDownSamples = 1;

		std::vector<Ref<RenderTarget2D>> m_GaussianSumTextures;
	};
}
