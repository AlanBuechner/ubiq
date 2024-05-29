#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class CommandList;
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

		void Submit(Ref<CommandQueue> queue);
		void OnViewportResize(uint32 width, uint32 height);
		void Build();

		Engine::Ref<Engine::FrameBuffer> GetRenderTarget();

		template<class T>
		T& As() { return *(T*)this; }

	protected:
		std::vector<Engine::Ref<RenderGraphNode>> m_Nodes;
		
		Engine::Ref<OutputNode> m_OutputNode;

		std::vector<Engine::Ref<Engine::CommandList>> m_CommandLists;
	};
}
