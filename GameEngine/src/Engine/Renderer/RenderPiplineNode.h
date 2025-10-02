#pragma once

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
	class RenderPiplineNode
	{
	public:

		virtual void Build() {};
		virtual void OnViewportResize(uint32 width, uint32 height) {}

		void SetCommandList(Ref<CPUCommandList> commandList) { m_CommandList = commandList; }
		Ref<CPUCommandList> GetCommandList() { return m_CommandList; }

	protected:

	protected:
		Ref<CPUCommandList> m_CommandList;
	};

	// output node
	class OutputNode : public RenderPiplineNode
	{
	public:

		virtual void Build() override;

		Ref<FrameBuffer> m_Buffer;
	};

	// frame buffer node
	class FrameBufferNode : public RenderPiplineNode
	{
	public:
		FrameBufferNode(const Utils::Vector<Ref<RenderTarget2D>>& fbSpec);
		
		virtual void OnViewportResize(uint32 width, uint32 height) override;

		Ref<FrameBuffer> m_Buffer;
	};

	// transition node
	class TransitionNode : public RenderPiplineNode
	{
	public:
		struct TransitionObject
		{
			GPUResourceHandle handle;
			ResourceState state;
		};

	public:

		void AddBuffer(const TransitionObject& transition);

		virtual void Build() override;


	private:
		Utils::Vector<TransitionObject> m_Transitions;
	};

	// resolve msaa node
	class ResolveMSAANode : public RenderPiplineNode
	{
	public:

		void SetDestination(Ref<FrameBuffer> buffer) { m_Dest = buffer; }
		void SetSource(Ref<FrameBuffer> buffer) { m_Src = buffer; }

		virtual void Build() override;

	private:
		Ref<FrameBuffer> m_Dest;
		Ref<FrameBuffer> m_Src;
	};



}
