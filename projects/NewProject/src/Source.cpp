#include <Reflection.h>
#include <Engine.h>
#include <Engine/Core/GameBase.h>
#include "Renderer/Components/CameraComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"

LINK_REFLECTION_DATA(NewProject)

namespace Game
{
	class Game : public Engine::GameBase
	{
	public:
		Game()
		{}

		virtual void OnUpdate(Engine::Ref<Engine::Camera> overideCamera) override
		{
			CREATE_PROFILE_FUNCTIONI();
			Engine::Ref<Engine::Camera> camera = (overideCamera ? overideCamera : GetSceneCamera());

			if(camera)
				GetScene()->OnUpdate(camera);
		}

		Engine::Ref<Engine::Camera> GetSceneCamera()
		{
			auto view = m_Scene->GetRegistry().View<CameraComponent>();
			for (auto comp : view)
				if (comp.Primary) return comp.Camera;
			return nullptr;
		}

		virtual void OnRender() override
		{
			CREATE_PROFILE_FUNCTIONI();
			Engine::Ref<Engine::CommandList> commandList = Engine::Renderer::GetMainCommandList();
			Engine::Ref<Engine::FrameBuffer> framBuffer = m_Scene->GetSceneRenderer()->GetRenderTarget();
			m_Scene->GetSceneRenderer()->Build();
			m_Scene->GetSceneRenderer()->Render(Engine::Renderer::GetMainCommandQueue());
		}

		virtual Engine::Ref<Engine::SceneRenderer> CreateSceneRenderer() override {
			return SceneRenderer::Create();
		}

		virtual void OnSceneLoad(Engine::Ref<Engine::Scene> scene) override
		{
			CREATE_PROFILE_FUNCTIONI();
			scene->AddUpdateEvent<Engine::ComponentUpdateEvent<Engine::TransformComponent>>("UpdateHierarchyGlobalTransform");
			scene->AddUpdateEvent<Engine::SceneUpdateEvent>("OnPreRender");
		}


	};
}

Engine::GameBase* CreateGame()
{
	return new Game::Game();
}