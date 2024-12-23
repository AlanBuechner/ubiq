﻿#include "EditorLayer.h"
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


		m_ContentPanel.SetDirectory(Engine::Application::Get().GetProject().GetAssetsDirectory());
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
			m_EditorCamera->OnUpdate();
			if (Engine::Input::GetMouseButtonPressed(Engine::MouseCode::LEFT_MOUSE) && !Engine::Input::GetKeyDown(Engine::KeyCode::ALT) && !ImGuizmo::IsOver())
			{
				bool inWindow;
				Engine::Entity entity = GetEntityAtMousePosition(inWindow);
				if (inWindow)
					m_HierarchyPanel.SelectEntity(entity);
			}

			if (Engine::Input::GetKeyPressed(Engine::KeyCode::SPACE) && Engine::Input::GetKeyDown(Engine::KeyCode::CONTROL))
				m_ContentPanel.SetActive(!m_ContentPanel.IsActive());
		}

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
		static bool dockSpace = true;

		if (dockSpace)
		{
			static bool dockSpaceOpen = true;
			static bool opt_fullscreen_persistant = true;
			static ImGuiDockNodeFlags opt_flags = ImGuiDockNodeFlags_None;
			bool opt_fullscreen = opt_fullscreen_persistant;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
			if (opt_fullscreen)
			{
				ImGuiViewport* viewport = ImGui::GetMainViewport();
				ImGui::SetNextWindowPos(viewport->Pos);
				ImGui::SetNextWindowSize(viewport->Size);
				ImGui::SetNextWindowViewport(viewport->ID);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
				ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
				window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
				window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
			}

			// When using ImGuiDockNodeFlags_PassthruDockspace, DockSpace() will render our background and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (opt_flags & ImGuiDockNodeFlags_PassthruCentralNode)// ImGuiDockNodeFlags_PassthruDockspace)
				window_flags |= ImGuiWindowFlags_NoBackground;

			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", &dockSpaceOpen, window_flags);
			ImGui::PopStyleVar();

			if (opt_fullscreen)
				ImGui::PopStyleVar(2);

			// Dock space
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("MyDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), opt_flags);
			}

			if (ImGui::BeginMenuBar())
			{
				if (ImGui::BeginMenu("File"))
				{
					// Disabling full screen would allow the window to be moved to the front of other windows, 
					// which we can't undo at the moment without finer window depth/z control.
					//ImGui::MenuItem("Fullscreen", NULL, &opt_fullscreen_persistant);

					if (ImGui::MenuItem("New", "Ctrl+N"))
					{
						NewScene();
					}

					ImGui::Separator();

					if (ImGui::MenuItem("Save Scene", "Ctrl+S"))
					{
						SaveScene();
					}

					if (ImGui::MenuItem("Save Scene As...", "Ctrl+Shift+S"))
					{
						SaveSceneAs();
					}

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

			m_HierarchyPanel.OnImGuiRender();
			m_ContentPanel.OnImGuiRender();
			
			UI_Viewport();

			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Engine::Event* e)
	{
		Super::OnEvent(e);

		m_EditorCamera->OnEvent(e);
		m_Game->OnEvent(e);

		Engine::EventDispatcher dispacher(e);
		dispacher.Dispatch<Engine::KeyPressedEvent>(BIND_EVENT_FN(&EditorLayer::OnKeyPressed));
	}

	void EditorLayer::NewScene()
	{
		m_Game->SwitchScene(Engine::Scene::Create());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_HierarchyPanel.SetContext(m_Game->GetScene());
	}

	void EditorLayer::DefaultScene()
	{
		m_Game->SwitchScene(Engine::Scene::CreateDefault());
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_HierarchyPanel.SetContext(m_Game->GetScene());
	}

	void EditorLayer::LoadScene(const fs::path& file)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_LoadedScene = file;
		//Engine::Ref<Engine::Scene> scene = Engine::Application::Get().GetAssetManager().GetAsset<Engine::Scene>(file);
		Engine::Ref<Engine::Scene> scene = Engine::Scene::Create(file);
		m_Game->GetScene()->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_Game->SwitchScene(scene);
		m_HierarchyPanel.SetContext(m_Game->GetScene());
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

			// imguizmo keyboard shortcuts
			switch (e->GetKeyCode())
			{
			case Engine::KeyCode::Q:
				m_GizmoType = -1;
				break;
			case Engine::KeyCode::G:
			case Engine::KeyCode::T:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case Engine::KeyCode::S:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case Engine::KeyCode::R:
				m_GizmoType = ImGuizmo::OPERATION::ROTATE;
				break;
			default:
				break;
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

		ImVec2 viewPortPanalSize = ImGui::GetContentRegionAvail();
		if (m_ViewPortSize != *(Math::Vector2*)&viewPortPanalSize)
			m_ViewPortSize = { viewPortPanalSize.x, viewPortPanalSize.y };

		ImGui::Image((ImTextureID)m_Game->GetScene()->GetSceneRenderer()->GetRenderTarget()->GetAttachment(0)->GetSRVDescriptor()->GetGPUHandlePointer(), viewPortPanalSize);
		if (ImGui::BeginDragDropTarget())
		{
			/*if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				bool inWindow;
				int entityID = GetEntityIDAtMousePosition(inWindow);
				if (entityID != -1)
				{
					Entity e = { (EntityType)entityID, m_ActiveScene.get() };
					fs::path path = (const wchar_t*)payload->Data;
					if (Texture2D::ValidExtention(path.extension().string()) && e.HasComponent<SpriteRendererComponent>())
						e.GetComponent<SpriteRendererComponent>().Texture = Application::Get().GetAssetManager().GetAsset<Texture2D>(path);
				}
			}*/
			ImGui::EndDragDropTarget();
		}

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

		// overlay
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
		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + viewPortPanalSize.x, minBound.y + viewPortPanalSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		Engine::Entity selected = m_HierarchyPanel.GetSelectedEntity();
		if (selected)
		{
			// TODO : Fix for child entity's
			// Gizmo's
			if (m_GizmoType != -1)
			{
				// camera editor
				const Math::Mat4& cameraProjection = m_EditorCamera->GetProjectionMatrix();
				Math::Mat4 cameraView = m_EditorCamera->GetViewMatrix();

				// transform
				auto& tc = selected.GetTransform(); // get the transform component
				Math::Mat4 transform = tc.GetGlobalTransform(); // get the transform matrix
				Math::Vector3 OldPosition, OldRotation, OldScale;
				Math::DecomposeTransform(transform, OldPosition, OldRotation, OldScale);

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);

				ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
					(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing())
				{
					Math::Vector3 position, rotation, scale;
					Math::DecomposeTransform(transform, position, rotation, scale);

					tc.Translate(position - OldPosition);
					tc.Rotate(rotation - OldRotation);
					tc.Scale(scale - OldScale);
				}
			}
		}

		ImGui::End();
		ImGui::PopStyleVar();
	}

	Engine::Entity EditorLayer::GetEntityAtMousePosition(bool& inWindow)
	{
		inWindow = false;

		auto [mx, my] = ImGui::GetMousePos();
		mx -= m_ViewportBounds[0].x;
		my -= m_ViewportBounds[0].y;
		Math::Vector2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];

		my = viewportSize.y - my; // flip y cord

		int mousex = (int)mx;
		int mousey = (int)my;

		if ((mousex >= 0 && mousex < (int)viewportSize.x) && (mousey >= 0 && mousey < (int)viewportSize.y))
		{
			inWindow = true;
			Engine::Ray ray;
			ray.m_Origin = m_EditorCamera->GetPosition();

			Math::Vector2 uv = Math::Vector2((float)mousex / viewportSize.x, (float)mousey / viewportSize.y);
			Math::Vector4 ndc = Math::Vector4(uv * 2.0f - 1.0f, 1, 1);
			CORE_INFO("{0}, {1}", ndc.x, ndc.y);
			Math::Vector4 worldSpace = Math::Inverse(m_EditorCamera->GetViewProjection()) * ndc;
			worldSpace = worldSpace / worldSpace.w;
			ray.m_Direction = Math::Vector3(worldSpace) - m_EditorCamera->GetPosition();

			Engine::Entity hitEntity;
			float closestHit = FLT_MAX;

			m_Game->GetScene()->GetRegistry().EachEntity([&](Engine::EntityType et){
				Engine::Entity e{ et, m_Game->GetScene().get() };
				Engine::AABB aabb = e.GetLocalAABB();
				if (aabb.m_Min != aabb.m_Max)
				{
					Engine::PlainVolume volume = e.GetPlainVolume();
					Engine::RayHit hit;
					if (volume.TestRay(ray, hit))
					{
						CORE_INFO("Hit {0}", e.GetName());
						if (hit.m_Distance < closestHit)
						{
							closestHit = hit.m_Distance;
							hitEntity = e;
						}
					}
				}
			});

			return hitEntity;
		}

		inWindow = false;
		return Engine::Entity::null;
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
