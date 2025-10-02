#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPiplineNode.h"
#include "RenderingUtils/RenderPassObject.h"

namespace Game
{
	class GBufferPassNode : public Engine::RenderPiplineNode
	{
	public:

		void SetCameraBind(Engine::Ref<Engine::Camera>* camera) { m_CameraBind = camera; }
		void SetDrawCommandsBind(std::vector<RenderPassDrawCommand>* drawCommands) { m_DrawCommandsBind = drawCommands; }

		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void Build() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;

		Engine::Ref<Engine::Camera>* m_CameraBind = nullptr;
		std::vector<RenderPassDrawCommand>* m_DrawCommandsBind = nullptr;
	};
}
