#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class SkyboxNode : public RenderGraphNode
	{
	public:
		SkyboxNode(RenderGraph& graph);

		void SetRenderTarget(FrameBufferVar var);

		virtual void AddToCommandQueue(Ref<ExecutionOrder> order) override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<CommandList> m_CommandList;
		FrameBufferVar m_RenderTarget;

		Ref<Mesh> m_SkyboxMesh;
		Ref<Shader> m_SkyboxShader;
	};
}
