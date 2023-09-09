#pragma once
#include "RenderGraphNode.h"
#include "Engine/Renderer/PostProcessing/PostProcess.h"

namespace Engine
{
	class PostProcessNode : public RenderGraphNode
	{
	public:
		PostProcessNode(RenderGraph& graph);

		void SetRenderTarget(Ref<RenderTarget2D> fb);
		void SetInput(const PostProcessInput& input) { m_Input = input; }
		void SetSrc(Ref<Texture2D> src) { m_Src = src; }
		void AddPostProcess(Ref<PostProcess> post) { m_PostProcessStack.push_back(post); }

		void InitPostProcessStack();

		virtual void OnViewportResize(uint32 width, uint32 height) override;

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<RenderTarget2D> m_RenderTarget;
		Ref<RenderTarget2D> m_BackBuffer;
		Ref<Texture2D> m_Src;

		std::vector<Ref<PostProcess>> m_PostProcessStack;

		Ref<Mesh> m_ScreenMesh;
		PostProcessInput m_Input;
	};
}
