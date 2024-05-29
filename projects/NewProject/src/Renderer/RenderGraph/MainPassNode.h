#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderGraphNode.h"

namespace Game
{
	class ShaderPassNode : public Engine::RenderGraphNode
	{
	public:
		ShaderPassNode(Engine::RenderGraph& graph, const std::string& passName);

		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void BuildImpl() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;
		std::string m_PassName;
	};
}
