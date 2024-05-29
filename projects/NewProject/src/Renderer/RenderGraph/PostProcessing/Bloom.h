#pragma once
#include "Engine/Core/Core.h"
#include "PostProcess.h"
#include "Engine/Renderer/Abstractions/Shader.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"

//#define USE_BLOOM_COMPUTE

namespace Game
{
	class Bloom : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Engine::Ref<Engine::CommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Engine::Ref<Engine::Shader> m_BloomShader;
		uint32 m_NumberDownSamples = 1;
#ifdef USE_BLOOM_COMPUTE
		std::vector<Engine::Ref<Engine::RWTexture2D>> m_GaussianSumTextures;
#else
		std::vector<Engine::Ref<Engine::RenderTarget2D>> m_GaussianSumTextures;
#endif
	};
}
