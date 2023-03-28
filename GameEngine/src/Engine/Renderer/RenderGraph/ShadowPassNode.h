#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraph.h"
#include "RenderGraphNode.h"
#include "Engine/Renderer/SceneRenderer.h"

namespace Engine
{
	class ShadowPassNode : public RenderGraphNode
	{
	public:
		ShadowPassNode(RenderGraph& graph);

		virtual void AddToCommandQueue(Ref<ExecutionOrder> order) override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<CommandList> m_CommandList;
	};
}
