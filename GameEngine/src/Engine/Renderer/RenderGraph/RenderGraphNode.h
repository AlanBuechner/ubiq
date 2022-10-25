#pragma once

#include "RenderGraph.h"
#include "Engine/Renderer/Renderer.h"
#include "Engine/Renderer/CommandQueue.h"
#include "Engine/Renderer/CommandList.h"

namespace Engine
{
	class RenderGraph;

	class RenderGraphNode
	{
	public:
		RenderGraphNode(RenderGraph& graph);

		void Build();
		virtual void OnViewportResize(uint32 width, uint32 height) {}
		virtual void AddToCommandQueue() {}
		void Invalidate() { m_Built = false; }

		virtual std::vector<Ref<CommandList>> GetCommandLists() { return {}; }

	protected:
		virtual void BuildImpl() {};

	protected:
		bool m_Built = false;
		RenderGraph& m_Graph;
	};

	class OutputNode : public RenderGraphNode
	{
	public:
		OutputNode(RenderGraph& graph);

		virtual void OnViewportResize(uint32 width, uint32 height) override;

		Ref<FrameBuffer> m_Buffer;
	};

	class FrameBufferNode : public RenderGraphNode
	{
	public:
		FrameBufferNode(RenderGraph& graph);
		
		virtual void OnViewportResize(uint32 width, uint32 height) override;

		Ref<FrameBuffer> m_Buffer;
	};

	class FrameBufferVar
	{
	public:
		FrameBufferVar() = default;
		FrameBufferVar(Ref<RenderGraphNode> input, Ref<FrameBuffer> var);
		Ref<FrameBuffer> GetVar();
		Ref<RenderGraphNode> GetInput() { return m_Input; }

	private:
		Ref<RenderGraphNode> m_Input;
		Ref<FrameBuffer> m_Var;
	};
}
