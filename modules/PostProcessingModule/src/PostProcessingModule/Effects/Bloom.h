#pragma once
#include "Engine/Core/Core.h"
#include "PostProcess.h"
#include "Engine/Renderer/Shaders/Shader.h"
#include "Engine/Renderer/Abstractions/Resources/ConstantBuffer.h"

//#define USE_BLOOM_COMPUTE

namespace Game
{
	class Bloom : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input) override;
		virtual void RecordCommands(Engine::Ref<Engine::CPUCommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh) override;

	private:
		Engine::Ref<Engine::Shader> m_BloomShader;
	};
}
