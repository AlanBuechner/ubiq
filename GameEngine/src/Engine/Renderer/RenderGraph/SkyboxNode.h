#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class SkyboxNode : public RenderGraphNode
	{
	public:
		SkyboxNode(RenderGraph& graph);

		void SetRenderTarget(Ref<FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_RenderTarget;

		Ref<Mesh> m_SkyboxMesh;
		Ref<Shader> m_SkyboxShader;
	};
}
