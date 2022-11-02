#pragma once
#include "Engine/Core/Core.h"
#include "RenderGraphNode.h"

namespace Engine
{
	class SkyboxNode : public RenderGraphNode
	{
	public:
		SkyboxNode(RenderGraph& graph, uint32 uSeg, uint32 vSeg);

		void SetRenderTarget(FrameBufferVar var);

		virtual void AddToCommandQueue() override;
		virtual std::vector<Ref<CommandList>> GetCommandLists() override { return { m_CommandList }; }

	protected:
		virtual void BuildImpl() override;

	private:
		uint32 m_USeg, m_VSeg;

		Ref<CommandList> m_CommandList;
		FrameBufferVar m_RenderTarget;

		Ref<Texture> m_SkyboxTexture;

		Ref<Mesh> m_SkyboxMesh;
		Ref<Shader> m_SkyboxShader;
	};
}
