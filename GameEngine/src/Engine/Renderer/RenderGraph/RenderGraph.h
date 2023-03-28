#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/FrameBuffer.h"
#include "Engine/Renderer/Shader.h"
#include "Engine/Renderer/Mesh.h"
#include "Engine/Renderer/InstanceBuffer.h"
#include "Engine/Renderer/ConstantBuffer.h"
#include "Engine/Renderer/Texture.h"
#include "Engine/Renderer/Light.h"

namespace Engine
{
	class RenderGraphNode;
	class OutputNode;

	class ExecutionOrder;
	class CommandQueue;

	struct DrawCommand
	{
		Ref<Shader> m_Shader;
		Ref<Mesh> m_Mesh;
		Ref<InstanceBuffer> m_InstanceBuffer;
	};

	struct SceneData
	{
		std::vector<DrawCommand> m_DrawCommands;
		Ref<Camera> m_MainCamera;
		Ref<DirectionalLight> m_DirectinalLight;
		Ref<Texture> m_Skybox;
	};

	class RenderGraph
	{

	public:
		RenderGraph();
		~RenderGraph();

		void Submit(Ref<CommandQueue> queue);
		void OnViewportResize(uint32 width, uint32 height);
		void Build();
		SceneData& GetScene() { return m_Scene; }

		Ref<FrameBuffer> GetRenderTarget();

	private:

	private:
		std::vector<Ref<RenderGraphNode>> m_Nodes;
		
		Ref<OutputNode> m_OutputNode;

		Ref<ExecutionOrder> m_Order;

		SceneData m_Scene;
	};
}
