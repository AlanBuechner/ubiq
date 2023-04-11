#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class GBufferPassNode : public RenderGraphNode
	{
	public:
		GBufferPassNode(RenderGraph& graph);

		void SetRenderTarget(Ref<FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_RenderTarget;
	};
}
