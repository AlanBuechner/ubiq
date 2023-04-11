#pragma once
#include "RenderGraphNode.h"
#include "Engine/Renderer/PostProcessing/PostProcess.h"

namespace Engine
{
	class PostProcessNode : public RenderGraphNode
	{
	public:
		PostProcessNode(RenderGraph& graph);

		void SetRenderTarget(Ref<FrameBuffer> fb);
		void SetInput(const PostProcessInput& input) { m_Input = input; }
		void AddPostProcess(Ref<PostProcess> post) { m_PostProcessStack.push_back(post); }


	protected:
		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_RenderTarget;
		Ref<FrameBuffer> m_BackBuffer;

		std::vector<Ref<PostProcess>> m_PostProcessStack;

		Ref<Mesh> m_ScreenMesh;
		PostProcessInput m_Input;
	};
}
