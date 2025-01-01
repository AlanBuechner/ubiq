#include "EditorLayer.h"
#include "EditorAssets.h"

#include "Engine/Core/Scene/SceneSerializer.h"
#include "Engine/Core/Scene/TransformComponent.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Cursor.h"
#include "Engine/Util/Performance.h"
#include "Engine/Util/PlatformUtils.h"
#include "Engine/Core/Scene/SceneScriptBase.h"

// temp
#include "Engine/Core/Scene/SceneRegistry.h"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <memory>

#include <windows.h>

Editor::EditorLayer* Editor::EditorLayer::s_Instance = nullptr;

namespace Editor
{

	EditorLayer::EditorLayer()
		: Super("EditorLayer")
	{
		if (s_Instance == nullptr)
			s_Instance = this;

		m_GridMesh.m_Vertices.Reserve((size_t)(m_GridLines + 1) * 3);
		m_GridMesh.m_Indices.Reserve((size_t)(m_GridLines + 1) * 4);
		for (uint32 i = 0; i <= m_GridLines; i++)
		{
			float posz = (m_GridLineOffset * i) - m_GridExtent;
			m_GridMesh.m_Vertices.Push({ { -m_GridExtent	,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });
			m_GridMesh.m_Vertices.Push({ { 0				,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, m_GridColor.w - (m_GridColor.w * (abs(posz) / m_GridExtent)) } });
			m_GridMesh.m_Vertices.Push({ { m_GridExtent	,0 , posz, 1 }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });

			m_GridMesh.m_Indices.Push((i * 3) + 0);
			m_GridMesh.m_Indices.Push((i * 3) + 1);
			m_GridMesh.m_Indices.Push((i * 3) + 1);
			m_GridMesh.m_Indices.Push((i * 3) + 2);
		}

		m_ViewPortSize = { Engine::Application::Get().GetWindow().GetWidth(), Engine::Application::Get().GetWindow().GetHeight() };

		m_Panels.Push(Engine::CreateRef<SceneHierarchyPanel>());
		m_Panels.Push(Engine::CreateRef<ContentBrowserPanel>());
	}

	void EditorLayer::OnAttach()
	{
		m_Game = CreateGame();
		DefaultScene();

		m_EditorCamera = Engine::CreateRef<Engine::EditorCamera>();
		m_EditorCamera->SetOrientation({ Math::Radians(360-25), Math::Radians(25) });
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();

		// resize
		uint32 width = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget()->GetAttachment(0)->GetResource()->GetWidth();
		uint32 height = m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget()->GetAttachment(0)->GetResource()->GetHeight();
		if (m_ViewPortSize.x > 0.0f && m_ViewPortSize.y > 0.0f &&
			(width != m_ViewPortSize.x || height != m_ViewPortSize.y))
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
			if (Engine::Input::GetMouseButtonPressed(Engine::MouseCode::LEFT_MOUSE) && !Engine::Input::GetKeyDown(Engine::KeyCode::ALT) && !ImGuizmo::IsOver())
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
		DrawCustomGizmo();
	}

	void EditorLayer::OnRender()
	{
		CREATE_PROFILE_FUNCTIONI();
		Engine::InstrumentationTimer timer = CREATE_PROFILEI();
		timer.Start("Recored Commands");

		m_Game->OnRender();

		Engine::Ref<Engine::CommandList> commandList = Engine::Renderer::GetMainCommandList();

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
		if (ImGui::BeginMenuBar())
		{
			if (ImGui::BeginMenu("File"))
			{
				if (ImGui::MenuItem("New", "Ctrl+N"))
					NewScene();
				ImGui::Separator();

				if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					SaveScene();

				if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					SaveSceneAs();

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

		UI_Viewport();
	}

	void EditorLayer::OnEvent(Engine::Event* e)
	{
		Super::OnEvent(e);

		m_EditorCamera->OnEvent(e);
		m_Game->OnEvent(e);

		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnEvent(e);
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
		//Engine::Ref<Engine::Scene> scene = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Scene>(file);
		Engine::Ref<Engine::Scene> scene = Engine::Scene::Create(file);
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_Game->SwitchScene(scene);
		for (uint32 i = 0; i < m_Panels.Count(); i++)
			m_Panels[i]->OnSceneChange(m_Game->GetScene());
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
					SaveSceneAs();
				else if (controlPressed)
					SaveScene();
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

	void EditorLayer::DrawCustomGizmo()
	{
		// draw grid lines
		{
			Math::Vector3 camPos = m_EditorCamera->GetPosition();

			Math::Mat4 matz =	glm::translate(Math::Mat4(1.0f), { camPos.x, 0, camPos.z - fmod(camPos.z, m_GridLineOffset) });
			Math::Mat4 matx =	glm::translate(Math::Mat4(1.0f), { camPos.x - fmod(camPos.x, m_GridLineOffset), 0, camPos.z })
								* glm::rotate(Math::Mat4(1.0f), glm::radians(90.0f), { 0,1,0 });

			Engine::DebugRenderer::DrawLineMesh(m_GridMesh, matz);
			Engine::DebugRenderer::DrawLineMesh(m_GridMesh, matx);
		}

		m_Game->DrawGizmos();

		// draw camera frustum
		
	}

	void EditorLayer::UI_Viewport()
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

		int mousex = (int)mx;
		int mousey = (int)my;

		pos = Math::Vector2((float)mousex / viewportSize.x, (float)mousey / viewportSize.y);
		return (mousex >= 0 && mousex < (int)viewportSize.x) && (mousey >= 0 && mousey < (int)viewportSize.y);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = Engine::FileDialogs::OpenFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
			LoadScene(filepath);
	}

	void EditorLayer::SaveScene()
	{
		if (!m_LoadedScene.empty())
		{
			CORE_INFO("Saving Scene: {0}", m_LoadedScene.string());
			Engine::SceneSerializer serializer(m_Game->GetScene());
			serializer.Serialize(m_LoadedScene.string());
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = Engine::FileDialogs::SaveFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
		{
			CORE_INFO("Saving Scene As: {0}", filepath);
			Engine::SceneSerializer serializer(m_Game->GetScene());
			serializer.Serialize(filepath);
		}
	}

}
