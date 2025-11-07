#include "EditorLayer.h"
#include "EditorAssets.h"

#include "Engine/Core/Scene/SceneSerializer.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Cursor.h"
#include "Utils/Performance.h"
#include "Engine/PlatformUtils/PlatformUtils.h"
#include "Engine/Core/Scene/SceneScriptBase.h"

#include <memory>

LINK_REFLECTION_DATA(EditorModule)

Engine::Layer* GetEditorLayer()
{
	return new Editor::EditorLayer();
}


Editor::EditorLayer* Editor::EditorLayer::s_Instance = nullptr;

namespace Editor
{

	class TextPanel : public GUI::PanelBase
	{
	public:

		virtual void Init() override
		{
			m_DrawData.color = { 0.2, 0.2, 0.2, 1 };
			m_DrawData.borderColor = { 0.3, 0.3, 0.3, 1 };
			m_DrawData.borderWidth = 0.002f;
			m_DrawData.borderHardness = 0.8f;

			m_Transform.position = { 0.25, 0.25 };
			m_Transform.size = { 0.5, 0.5 };
		}
		

		virtual void OnGUIRender() override
		{
			GUI::PanelBase::OnGUIRender();
		}
	};




	EditorLayer::EditorLayer()
		: Super("EditorLayer")
	{
		if (s_Instance == nullptr)
			s_Instance = this;

		EditorAssets::Init();
		m_ViewPortSize = { Engine::Application::Get().GetWindow().GetWidth(), Engine::Application::Get().GetWindow().GetHeight() };

		GUI::Initalize();
		GUI::PanelRef panel1 = m_Context.CreatePanel<TextPanel>();
		GUI::PanelRef panel2 = m_Context.CreatePanel<TextPanel>();
		GUI::PanelRef panel3 = m_Context.CreatePanel<TextPanel>();

		panel1->Split(panel2, GUI::SplitAxis::X);
		panel2->Split(panel3, GUI::SplitAxis::Y);
	}

	EditorLayer::~EditorLayer()
	{
		EditorAssets::Destroy();
		GUI::Destroy();
	}

	void EditorLayer::OnAttach()
	{
		m_Game = CreateGame();
		DefaultScene();

		m_EditorCamera = Engine::CreateRef<Editor::EditorCamera>();
		m_EditorCamera->SetOrientation({ Math::Radians<float>(360-25), Math::Radians<float>(25) });
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();

		m_Context.SetWindowSize(Engine::Application::Get().GetWindow().GetWidth(), Engine::Application::Get().GetWindow().GetHeight());
		m_Context.OnGUIRender();


		// resize
		Engine::Ref<Engine::Texture2D> res = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget()->GetAttachment(0);
		if (m_ViewPortSize.x > 0.0f && m_ViewPortSize.y > 0.0f &&
			(res->GetWidth() != m_ViewPortSize.x || res->GetHeight() != m_ViewPortSize.y))
		{
			m_EditorCamera->SetViewportSize(m_ViewPortSize.x, m_ViewPortSize.y);
			m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		}

		// update scene
		if (!m_Playing)
		{
			// update editor camera
			m_EditorCamera->OnUpdate();

			// check if the screen was clicked
			if (Engine::Input::GetMouseButtonPressed(Engine::MouseCode::LEFT_MOUSE) && !Engine::Input::GetKeyDown(Engine::KeyCode::ALT))
			{
				Math::Vector2 pos;
				if (GetMousePositionInViewport(pos))
				{
					
				}
			}
		}


		// update game
		m_Game->OnUpdate(m_Playing ? nullptr : m_EditorCamera);
	}

	void EditorLayer::OnRender()
	{
		CREATE_PROFILE_FUNCTIONI();
		Profiler::InstrumentationTimer timer = CREATE_PROFILEI();
		START_PROFILEI(timer, "Recored Commands");

		m_Game->OnRender();

		Engine::Ref<Engine::CPUCommandList> commandList = Engine::Renderer::GetMainCommandList();

		Engine::Ref<Engine::FrameBuffer> framBuffer = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget();
		Engine::GPUTimer::BeginEvent(commandList, "gizmo's");
		commandList->SetRenderTarget(framBuffer);
		Engine::Renderer::Build(commandList);
		commandList->Present(framBuffer); // present the render target
		Engine::GPUTimer::EndEvent(commandList);


		Engine::GPUTimer::BeginEvent(commandList, "GUI");

		Engine::Ref<Engine::RenderTarget2D> rt = Engine::Application::Get().GetWindow().GetSwapChain()->GetCurrentRenderTarget();

		Engine::Ref<Engine::RenderTarget2D> depthBuffer = Engine::RenderTarget2D::Create(rt->GetWidth(), rt->GetHeight(),
			Engine::TextureFormat::Depth, { 1,0,0,0 }, Engine::ResourceCapabilities::Transient);

		commandList->AllocateTransient(depthBuffer);
		commandList->ClearRenderTarget(depthBuffer);

		m_Context.SetRenderTarget(rt);
		m_Context.SetDepthBuffer(depthBuffer);

		m_Context.Draw(commandList);
		m_Context.EndFrame();

		commandList->CloseTransient(depthBuffer);
		Engine::GPUTimer::EndEvent(commandList);

		timer.End();
	}

	void EditorLayer::NewScene()
	{
		m_Game->SwitchScene(Engine::Scene::Create());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
	}

	void EditorLayer::DefaultScene()
	{
		m_Game->SwitchScene(Engine::Scene::CreateDefault());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
	}

	void EditorLayer::LoadScene(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_LoadedScene = file;
		Engine::Ref<Engine::Scene> scene = Engine::Scene::Create(file);
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_Game->SwitchScene(scene);
	}

	

	bool EditorLayer::GetMousePositionInViewport(Math::Vector2& pos)
	{
		auto [mx, my] = std::pair(0,0); // TODO get mouse position
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;

		Math::Vector2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
		pos = Math::Vector2(mx / viewportSize.x, my / viewportSize.y);
		return (mx >= 0 && mx < viewportSize.x) && (my >= 0 && my < viewportSize.y);
	}

	void EditorLayer::OpenSceneDialog()
	{
		std::string filepath = Engine::FileDialogs::OpenFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
			LoadScene(filepath);
	}

	void EditorLayer::SaveSceneDialog()
	{
		if (!m_LoadedScene.empty())
		{
			CORE_INFO("Saving Scene: {0}", m_LoadedScene.string());
			Engine::SceneSerializer serializer(m_Game->GetScene());
			serializer.Serialize(m_LoadedScene.string());
		}
	}

	void EditorLayer::SaveSceneAsDialog()
	{
		std::string filepath = Engine::FileDialogs::SaveFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
		{
			CORE_INFO("Saving Scene As: {0}", filepath);
			Engine::SceneSerializer serializer(m_Game->GetScene());
			serializer.Serialize(filepath);
		}
	}


	void EditorLayer::OnEvent(Engine::Event* e)
	{
		Super::OnEvent(e);

		if (m_Context.OnEvent(e))
			return;

		m_EditorCamera->OnEvent(e);
		m_Game->OnEvent(e);
	}

	bool EditorLayer::OnKeyPressed(Engine::KeyPressedEvent* e)
	{
		bool controlPressed = Engine::Input::GetKeyDown(Engine::KeyCode::CONTROL);
		bool shiftPressed = Engine::Input::GetKeyDown(Engine::KeyCode::SHIFT);
		bool rightClick = Engine::Input::GetMouseButtonDown(Engine::MouseCode::RIGHT_MOUSE);

		if (!rightClick)
		{
			switch (e->GetKeyCode())
			{
			case Engine::KeyCode::S:
			{
				if (controlPressed && shiftPressed)
					SaveSceneAsDialog();
				else if (controlPressed)
					SaveSceneDialog();
				break;
			}
			case Engine::KeyCode::N:
			{
				if (controlPressed)
					NewScene();
				break;
			}
			}
		}

		return true;
	}

}
