#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderGraphNode.h"

namespace Game
{
	class GBufferPassNode : public Engine::RenderGraphNode
	{
	public:
		GBufferPassNode(Engine::RenderGraph& graph);

		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;
	};
}
