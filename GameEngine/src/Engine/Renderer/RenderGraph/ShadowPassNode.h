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

	protected:
		virtual void BuildImpl() override;

	};
}
