#pragma once

#include "RenderGraph.h"
#include "Engine/Renderer/Abstractions/CommandList.h"

namespace Engine
{
	class CPUCommandList;

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

		void SetCommandList(Ref<CPUCommandList> commandList) { m_CommandList = commandList; }
		Ref<CPUCommandList> GetCommandList() { return m_CommandList; }

		void AddDependincy(Ref<RenderGraphNode> dependency) { m_Dependincys.Push(dependency); }

	protected:
		virtual void BuildImpl() {};

	protected:
		bool m_Built = false;
		RenderGraph& m_Graph;

		Utils::Vector<Ref<RenderGraphNode>> m_Dependincys;
		Ref<CPUCommandList> m_CommandList;
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

	// transition node
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
		Utils::Vector<TransitionObject> m_Transitions;
	};

	// resolve msaa node
	class ResolveMSAANode : public RenderGraphNode
	{
	public:
		ResolveMSAANode(RenderGraph& graph);

		void SetDestination(Ref<FrameBuffer> buffer) { m_Dest = buffer; }
		void SetSource(Ref<FrameBuffer> buffer) { m_Src = buffer; }

		virtual void BuildImpl() override;

	private:
		Ref<FrameBuffer> m_Dest;
		Ref<FrameBuffer> m_Src;
	};



}
