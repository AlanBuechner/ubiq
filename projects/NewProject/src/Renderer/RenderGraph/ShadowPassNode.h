#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderGraph.h"
#include "Engine/Renderer/RenderGraphNode.h"
#include "Engine/Renderer/SceneRenderer.h"

namespace Game
{
	class ShadowPassNode : public Engine::RenderGraphNode
	{
	public:
		ShadowPassNode(Engine::RenderGraph& graph);

	protected:
		virtual void BuildImpl() override;

	};
}
