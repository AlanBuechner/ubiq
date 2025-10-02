#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/RenderPiplineNode.h"

namespace Engine
{
	class Camera;
}

namespace Game
{
	class SkyboxNode : public Engine::RenderPiplineNode
	{
	public:
		SkyboxNode();

		void SetCameraBind(Engine::Ref<Engine::Camera>* camera) { m_CameraBind = camera; }
		void SetSkyboxTextureBind(Engine::Ref<Engine::Texture2D>* texture) { m_SkyboxTextureBind = texture; }
		void SetRenderTarget(Engine::Ref<Engine::FrameBuffer> fb) { m_RenderTarget = fb; }

	protected:
		virtual void Build() override;

	private:
		Engine::Ref<Engine::FrameBuffer> m_RenderTarget;

		Engine::Ref<Engine::Mesh> m_SkyboxMesh;
		Engine::Ref<Engine::Shader> m_SkyboxShader;

		Engine::Ref<Engine::Camera>* m_CameraBind = nullptr;
		Engine::Ref<Engine::Texture2D>* m_SkyboxTextureBind = nullptr;
	};
}
