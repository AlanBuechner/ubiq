#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPiplineNode.h"
#include "RenderingUtils/RenderPassObject.h"
#include "RenderingUtils/Lighting/DirectionalLight.h"

namespace Game
{
	class ShaderPassNode : public Engine::RenderPiplineNode
	{
	public:
		ShaderPassNode(const std::string& passName);

		void SetCameraBind(Engine::Ref<Engine::Camera>* camera) { m_CameraBind = camera; }
		void SetDrawCommandsBind(std::vector<RenderPassDrawCommand>* drawCommands) { m_DrawCommandsBind = drawCommands; }
		void SetDirectionalLightBind(Engine::Ref<DirectionalLight>* light) { m_DirLightBind = light; }

		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void Build() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;
		std::string m_PassName;

		Engine::Ref<Engine::Camera>* m_CameraBind = nullptr;
		std::vector<RenderPassDrawCommand>* m_DrawCommandsBind = nullptr;
		Engine::Ref<DirectionalLight>* m_DirLightBind = nullptr;
	};
}
