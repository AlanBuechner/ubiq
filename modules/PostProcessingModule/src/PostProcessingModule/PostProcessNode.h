#pragma once
#include "Engine/Renderer/RenderPiplineNode.h"
#include "Effects/PostProcess.h"

namespace Game
{
	class PostProcessNode : public Engine::RenderPiplineNode
	{
	public:
		PostProcessNode();

		void SetRenderTarget(Engine::Ref<Engine::RenderTarget2D> fb);
		void SetInput(const PostProcessInput& input) { m_Input = input; }
		void SetSrc(Engine::Ref<Engine::Texture2D> src) { m_Src = src; }
		void AddPostProcess(Engine::Ref<PostProcess> post) { m_PostProcessStack.push_back(post); }

		void InitPostProcessStack();

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	protected:
		virtual void Build() override;

	private:
		Engine::Ref<Engine::RenderTarget2D> m_RenderTarget;
		Engine::Ref<Engine::RenderTarget2D> m_BackBuffer;
		Engine::Ref<Engine::Texture2D> m_Src;

		std::vector<Engine::Ref<PostProcess>> m_PostProcessStack;

		Engine::Ref<Engine::Mesh> m_ScreenMesh;
		PostProcessInput m_Input;
	};
}
