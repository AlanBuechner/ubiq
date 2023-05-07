#pragma once
#include "Engine/Core/Core.h"
#include "PostProcess.h"
#include "Engine/Renderer/Shader.h"


namespace Engine
{
	class DepthOfField : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<FrameBuffer> renderTarget, uint64 srcDescriptorLocation, const PostProcessInput& input, Ref<Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Ref<Shader> m_DepthOfFieldShader;
		Ref<FrameBuffer> m_TempTexture;
		Ref<FrameBuffer> m_COCTexture;

		Ref<FrameBuffer> m_FarBlur;
		Ref<FrameBuffer> m_NearBlur;

		SceneData* m_Scene;

		float m_FocalPlane = 50;
		float m_ConfusionRadius = 200;
		float m_COCBlurRadius = 0.019f;
		float m_BokehStrangth = 0.7f;
	};
}
