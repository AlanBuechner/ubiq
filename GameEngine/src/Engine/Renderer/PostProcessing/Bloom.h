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
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Ref<Shader> m_BloomShader;
		uint32 m_NumberDownSamples = 6;

		std::vector<Ref<FrameBuffer>> m_GaussianSumBuffers;
	};
}
