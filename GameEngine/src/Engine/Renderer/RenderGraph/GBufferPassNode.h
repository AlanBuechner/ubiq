#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class GBufferPassNode : public RenderGraphNode
	{
	public:
		GBufferPassNode(RenderGraph& graph);

		void SetRenderTarget(FrameBufferVar var);

		virtual void AddToCommandQueue() override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<CommandList> m_CommandList;
		FrameBufferVar m_RenderTarget;
	};
}
