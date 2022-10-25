#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class ShaderPassNode : public RenderGraphNode
	{
	public:
		ShaderPassNode(RenderGraph& graph, const std::string& passName);

		void SetRenderTarget(FrameBufferVar var);

		virtual void AddToCommandQueue() override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

	protected:
		virtual void BuildImpl() override;

	private:
		Ref<CommandList> m_CommandList;
		FrameBufferVar m_RenderTarget;
		std::string m_PassName;
	};
}
