#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class CPUCommandList;
	class DirectionalLight;
	class Texture2D;
	class ConstantBuffer;
	class InstanceBuffer;
	class Mesh;
	class Shader;
	class FrameBuffer;
	class Camera;
}

namespace Engine
{
	class RenderGraphNode;
	class OutputNode;

	class ExecutionOrder;
	class CommandQueue;


	class RenderGraph
	{

	public:
		RenderGraph();
		~RenderGraph();

		void Submit();
		void OnViewportResize(uint32 width, uint32 height);
		void Build();

		Ref<FrameBuffer> GetRenderTarget();

		template<class T>
		T& As() { return *(T*)this; }

	protected:
		Utils::Vector<Ref<RenderGraphNode>> m_Nodes;
		
		Ref<OutputNode> m_OutputNode;

		Utils::Vector<Ref<CPUCommandList>> m_CommandLists;
	};
}
