#pragma once
#include "Engine/Renderer/RenderGraph.h"

namespace Engine
{
	class VertexBuffer;
}

namespace Game
{
	class DirectionalLight;
}

namespace Game
{
	struct DrawCommand
	{
		Engine::Ref<Engine::Shader> m_Shader;
		Engine::Ref<Engine::Mesh> m_Mesh;
		Engine::Ref<Engine::VertexBuffer> m_InstanceBuffer;
	};

	struct SceneData
	{
		std::vector<DrawCommand> m_DrawCommands;
		Engine::Ref<Engine::Camera> m_MainCamera;
		Engine::Ref<DirectionalLight> m_DirectinalLight;
		Engine::Ref<Engine::Texture2D> m_Skybox;
	};

	class RenderGraph : public Engine::RenderGraph
	{
	public:
		RenderGraph();
		SceneData& GetScene() { return m_Scene; }

	private:
		SceneData m_Scene;
	};
}
