#pragma once
#include "RenderGraphNode.h"
#include "Engine/Renderer/PostProcessing/PostProcess.h"

namespace Engine
{
	class PostProcessNode : public RenderGraphNode
	{
	public:
		PostProcessNode(RenderGraph& graph);

		void SetRenderTarget(FrameBufferVar var);

		virtual void AddToCommandQueue(Ref<ExecutionOrder> order) override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

		void AddPostProcess(Ref<PostProcess> post) { m_PostProcessStack.push_back(post); }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<CommandList> m_CommandList;
		FrameBufferVar m_RenderTarget;
		Ref<FrameBuffer> m_BackBuffer;

		std::vector<Ref<PostProcess>> m_PostProcessStack;

		Ref<Mesh> m_ScreenMesh;
	};
}
