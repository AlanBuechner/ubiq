#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderGraphNode.h"

namespace Game
{
	class SkyboxNode : public Engine::RenderGraphNode
	{
	public:
		SkyboxNode(Engine::RenderGraph& graph);

		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;

		Engine::Ref<Engine::Mesh> m_SkyboxMesh;
		Engine::Ref<Engine::Shader> m_SkyboxShader;
	};
}
