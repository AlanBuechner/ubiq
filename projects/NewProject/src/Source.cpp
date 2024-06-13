#include <Reflection.h>
#include <Engine.h>
#include <Engine/Core/GameBase.h>
#include "Renderer/Components/CameraComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"
#include "SceneScripts/DefaultSceneScript.h"

LINK_REFLECTION_DATA(NewProject)

namespace Game
{
	class Game : public Engine::GameBase
	{
	public:
		Game()
		{
			m_DefaultCamera = Engine::CreateRef<Engine::EditorCamera>();
			m_DefaultCamera->SetOrientation({ Math::Radians(180 - 25), Math::Radians(25) });
		}

		virtual void OnUpdate(Engine::Ref<Engine::Camera> overideCamera) override
		{
			CREATE_PROFILE_FUNCTIONI();
			Engine::Ref<Engine::Camera> camera = (overideCamera ? overideCamera : GetSceneCamera());

			if (camera == m_DefaultCamera)
				m_DefaultCamera->OnUpdate();

			if(camera)
				GetScene()->OnUpdate(camera);
		}

		Engine::Ref<Engine::Camera> GetSceneCamera()
		{
			auto view = m_Scene->GetRegistry().View<CameraComponent>();
			for (auto comp : view)
				if (comp.Primary) return comp.Camera;
			return m_DefaultCamera;
		}

		virtual void OnRender() override
		{
			CREATE_PROFILE_FUNCTIONI();
			Engine::Ref<Engine::CommandList> commandList = Engine::Renderer::GetMainCommandList();
			Engine::Ref<Engine::FrameBuffer> framBuffer = m_Scene->GetSceneRenderer()->GetRenderTarget();
			m_Scene->GetSceneRenderer()->Build();
			m_Scene->GetSceneRenderer()->Render(Engine::Renderer::GetMainCommandQueue());
		}

	private:
		Engine::Ref<Engine::EditorCamera> m_DefaultCamera;

	};
}

Engine::GameBase* CreateGame()
{
	return new Game::Game();
}
