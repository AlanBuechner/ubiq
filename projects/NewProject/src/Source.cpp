#include <Reflection.h>
#include <Engine.h>
#include <Engine/Core/GameBase.h>
#include "Renderer/Components/CameraComponent.h"
#include "Renderer/SceneRenderer/SceneRenderer.h"
#include "SceneScripts/DefaultSceneScript.h"
#include "TestModule/src/TestModule.h"
#include "Renderer/FreeCamera.h"


LINK_REFLECTION_DATA(NewProject)

#ifndef EDITOR

Engine::Layer* GetEditorLayer()
{
	return nullptr;
}

#endif

namespace Game
{
	class Game : public Engine::GameBase
	{
	public:
		Game()
		{
			m_DefaultCamera = Engine::CreateRef<FreeCamera>();
			m_DefaultCamera->SetOrientation({ Math::Radians<float>(180 - 25), Math::Radians<float>(25) });
			TestModule::TestModule();
		}

		virtual void OnEvent(Engine::Event* e) override
		{
			m_Scene->GetSceneScript()->OnEvent(e);

			if(!e->Handled)
				m_DefaultCamera->OnEvent(e);
		}

		virtual void OnUpdate(Engine::Ref<Engine::Camera> overideCamera) override
		{
			CREATE_PROFILE_FUNCTIONI();
			Engine::Ref<Engine::Camera> camera = (overideCamera ? overideCamera : GetSceneCamera());

			Engine::DebugRenderer::SetCamera(*camera);

			if (camera == m_DefaultCamera)
				m_DefaultCamera->OnUpdate();

			if (!m_Scene->GetSceneRenderer()->GetCameras().Contains(camera))
				m_Scene->GetSceneRenderer()->AddCamera(camera);

			if(camera)
				GetScene()->OnUpdate();


			m_Scene->GetSceneRenderer()->SetMainCamera(camera);
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
			Engine::Ref<Engine::CPUCommandList> commandList = Engine::Renderer::GetMainCommandList();
			Engine::Ref<Engine::FrameBuffer> framBuffer = m_Scene->GetSceneRenderer()->GetRenderTarget();
			m_Scene->GetSceneRenderer()->Build();
		}

	private:
		Engine::Ref<FreeCamera> m_DefaultCamera;

	};
}

Engine::GameBase* CreateGame()
{
	return new Game::Game();
}
