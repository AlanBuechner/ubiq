#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class ShaderPassNode : public RenderGraphNode
	{
	public:
		ShaderPassNode(RenderGraph& graph, const std::string& passName);

		void SetRenderTarget(Ref<FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_RenderTarget;
		std::string m_PassName;
	};
}
