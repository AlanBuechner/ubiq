#pragma once
#include "Engine/Renderer/RenderGraph.h"
#include "RenderingUtils/RenderPassObject.h"

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

	struct SceneData
	{
		std::vector<RenderPassDrawCommand> m_MainPassDrawCommands;
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
