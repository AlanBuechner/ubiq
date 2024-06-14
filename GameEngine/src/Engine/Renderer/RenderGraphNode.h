#pragma once

#include "RenderGraph.h"
#include "Engine/Renderer/Abstractions/CommandList.h"

namespace Engine
{
	class CommandList;

	struct FrameBufferSpecification;
}

namespace Engine
{
	class RenderGraph;

	// base node
	class RenderGraphNode
	{
	public:
		RenderGraphNode(RenderGraph& graph);

		void Build();
		virtual void OnViewportResize(uint32 width, uint32 height) {}
		virtual void Invalidate() { m_Built = false; }

		void SetCommandList(Ref<CommandList> commandList) { m_CommandList = commandList; }
		Ref<CommandList> GetCommandList() { return m_CommandList; }

		void AddDependincy(Ref<RenderGraphNode> dependency) { m_Dependincys.push_back(dependency); }

	protected:
		virtual void BuildImpl() {};

	protected:
		bool m_Built = false;
		RenderGraph& m_Graph;

		std::vector<Ref<RenderGraphNode>> m_Dependincys;
		Ref<CommandList> m_CommandList;
	};

	// nodes
	// output node
	class OutputNode : public RenderGraphNode
	{
	public:
		OutputNode(RenderGraph& graph);

		virtual void BuildImpl() override;

		Ref<FrameBuffer> m_Buffer;
	};

	// frame buffer node
	class FrameBufferNode : public RenderGraphNode
	{
	public:
		FrameBufferNode(RenderGraph& graph, const Utils::Vector<Ref<RenderTarget2D>>& fbSpec);
		
		virtual void OnViewportResize(uint32 width, uint32 height) override;

		Ref<FrameBuffer> m_Buffer;
	};

	// transition Node
	class TransitionNode : public RenderGraphNode
	{
	public:
		struct TransitionObject
		{
			GPUResourceHandle handle;
			ResourceState state;
		};

	public:
		TransitionNode(RenderGraph& graph);

		void AddBuffer(const TransitionObject& transition);

		virtual void BuildImpl() override;


	private:
		std::vector<TransitionObject> m_Transitions;
	};
}
