#pragma once
#include "Engine/Core/Core.h"
#include "PostProcess.h"
#include "Engine/Renderer/Abstractions/Shader.h"


namespace Game
{
	class DepthOfField : public PostProcess
	{
	public:
		virtual void Init(const PostProcessInput& input, SceneData& scene) override;
		virtual void RecordCommands(Engine::Ref<Engine::CommandList> commandList, Engine::Ref<Engine::RenderTarget2D> renderTarget, Engine::Ref<Engine::Texture2D> src, const PostProcessInput& input, Engine::Ref<Engine::Mesh> screenMesh) override;

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	private:
		Engine::Ref<Engine::Shader> m_DepthOfFieldShader;

		Engine::Ref<Engine::RenderTarget2D> m_TempTexture;
		Engine::Ref<Engine::RenderTarget2D> m_COCTexture;
		Engine::Ref<Engine::RenderTarget2D> m_FarBlur;
		Engine::Ref<Engine::RenderTarget2D> m_NearBlur;
		Engine::Ref<Engine::FrameBuffer> m_BokehBlur;

		float m_FocalPlane = 50;
		float m_ConfusionRadius = 200;
		float m_COCBlurRadius = 0.019f;
		float m_BokehStrangth = 0.3f;
	};
}
