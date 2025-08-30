#include "EditorLayer.h"
#include "EditorAssets.h"

#include "Engine/Core/Scene/SceneSerializer.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Cursor.h"
#include "Utils/Performance.h"
#include "Engine/PlatformUtils/PlatformUtils.h"
#include "Engine/Core/Scene/SceneScriptBase.h"

#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Editor/Panels/GridGizmosPanel.h"

#include <imgui/imgui.h>
#include <memory>

LINK_REFLECTION_DATA(EditorModule)

Engine::Layer* GetEditorLayer()
{
	return new Editor::EditorLayer();
}


Editor::EditorLayer* Editor::EditorLayer::s_Instance = nullptr;

namespace Editor
{

	EditorLayer::EditorLayer()
		: Super("EditorLayer")
	{
		if (s_Instance == nullptr)
			s_Instance = this;

		EditorAssets::Init();
		m_ViewPortSize = { Engine::Application::Get().GetWindow().GetWidth(), Engine::Application::Get().GetWindow().GetHeight() };

		m_Panels.Push(Engine::CreateRef<SceneHierarchyPanel>());
		m_Panels.Push(Engine::CreateRef<ContentBrowserPanel>());
		m_Panels.Push(Engine::CreateRef<GridGizmosPanel>());
	}

	EditorLayer::~EditorLayer()
	{
		EditorAssets::Destroy();
	}

	void EditorLayer::OnAttach()
	{
		m_Game = CreateGame();
		DefaultScene();

		m_EditorCamera = Engine::CreateRef<Editor::EditorCamera>();
		m_EditorCamera->SetOrientation({ Math::Radians(360-25), Math::Radians(25) });
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();

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
					for (uint32 i = 0; i < m_Panels.Count(); i++)
						m_Panels[i]->OnScreenClick(pos);
				}
			}
		}

		// update editor panels
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnUpdate();

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


		timer.End();
	}

	void EditorLayer::OnImGuiRender()
	{
		// draw menu
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();
				ImGui::Separator();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveSceneDialog();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAsDialog();

				if (ImGui::MenuItem("Exit"))
					Engine::Application::Get().Close();

				ImGui::EndMenu();
			}

			fs::path tempScenePath = fs::current_path() / "temp/tempScene.ubiq";
			if (ImGui::MenuItem(m_Playing ? "Stop" : "Play"))
			{
				// save scene to temp file
				if (m_Playing == false)
				{
					Engine::SceneSerializer serializer(m_Game->GetScene());
					serializer.Serialize(tempScenePath);
				}
				m_Playing = !m_Playing;
				LoadScene(tempScenePath);
			}

			ImGui::EndMenuBar();
		}

		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnImGuiRender();

		DrawViewport();
		m_Game->DrawGizmos();
	}

	void EditorLayer::NewScene()
	{
		m_Game->SwitchScene(Engine::Scene::Create());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnSceneChange(m_Game->GetScene());
	}

	void EditorLayer::DefaultScene()
	{
		m_Game->SwitchScene(Engine::Scene::CreateDefault());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnSceneChange(m_Game->GetScene());
	}

	void EditorLayer::LoadScene(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_LoadedScene = file;
		Engine::Ref<Engine::Scene> scene = Engine::Scene::Create(file);
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_Game->SwitchScene(scene);
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnSceneChange(m_Game->GetScene());
	}

	void EditorLayer::DrawViewport()
	{
		// Game window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport##Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);

		auto viewportOffset = ImGui::GetCursorPos();

		Engine::Application::Get().GetImGuiLayer()->SetBlockEvents(!ImGui::IsWindowFocused());

		// draw viewport
		ImVec2 viewPortPanalSize = ImGui::GetContentRegionAvail();
		if (m_ViewPortSize != *(Math::Vector2*)&viewPortPanalSize)
			m_ViewPortSize = { viewPortPanalSize.x, viewPortPanalSize.y };
		ImGui::Image((ImTextureID)m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget()->GetAttachment(0)->GetSRVDescriptor()->GetGPUHandlePointer(), viewPortPanalSize);

		// update bounds
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + viewPortPanalSize.x, minBound.y + viewPortPanalSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		// set cursor visibility
		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			Engine::Cursor::Visibility(false);
			Engine::Cursor::Lock(true);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			Engine::Cursor::Visibility(true);
			Engine::Cursor::Lock(false);
		}


		// draw overlay
		float fps = Time::GetFPS();
		ImGui::SetCursorPos(ImVec2(10, viewportOffset.y + 10));
		ImGui::PushStyleColor(ImGuiCol_Text, fps > 60.0f ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
		ImGui::Text("  FPS : %.2f", fps);
		ImGui::PopStyleColor();

		float frameTime = Time::GetDeltaMilliseconds();
		ImGui::SetCursorPosX(10);
		ImGui::PushStyleColor(ImGuiCol_Text, frameTime < 16.666f ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 0, 0, 255));
		ImGui::Text("Frame : %.2f m/s", Time::GetDeltaMilliseconds());
		ImGui::PopStyleColor();


		// gizmos
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnDrawGizmos();

		ImGui::End();
		ImGui::PopStyleVar();
	}

	bool EditorLayer::GetMousePositionInViewport(Math::Vector2& pos)
	{
		auto [mx, my] = ImGui::GetMousePos();
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

		m_EditorCamera->OnEvent(e);
		m_Game->OnEvent(e);

		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnEvent(e);
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
