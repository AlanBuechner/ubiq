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
		virtual void RecordCommands(Ref<CommandList> commandList, Ref<RenderTarget2D> renderTarget, Ref<Texture2D> src, const PostProcessInput& input, Ref<Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Ref<Shader> m_DepthOfFieldShader;

		Ref<RenderTarget2D> m_TempTexture;
		Ref<RenderTarget2D> m_COCTexture;
		Ref<RenderTarget2D> m_FarBlur;
		Ref<RenderTarget2D> m_NearBlur;

		SceneData* m_Scene;

		float m_FocalPlane = 50;
		float m_ConfusionRadius = 200;
		float m_COCBlurRadius = 0.019f;
		float m_BokehStrangth = 0.7f;
	};
}
