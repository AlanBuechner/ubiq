#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPiplineNode.h"
#include "RenderingUtils/RenderPassObject.h"
#include "RenderingUtils/Lighting/DirectionalLight.h"

namespace Game
{
	class ShadowPassNode : public Engine::RenderPiplineNode
	{
	public:
		void SetDirectionalLightBind(Engine::Ref<DirectionalLight>* light) { m_DirLightBind = light; }
		void SetDrawCommandsBind(std::vector<RenderPassDrawCommand>* drawCommands) { m_DrawCommandsBind = drawCommands; }

	protected:
		virtual void Build() override;

	private:
		Engine::Ref<DirectionalLight>* m_DirLightBind = nullptr;
		std::vector<RenderPassDrawCommand>* m_DrawCommandsBind = nullptr;
	};
}
