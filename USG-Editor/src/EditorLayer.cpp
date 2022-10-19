#include "EditorLayer.h"

#include "Engine/Core/Scene/SceneSerializer.h"
#include "Engine/Renderer/SceneRendererComponents.h"
#include "Engine/Math/Math.h"
#include "Engine/Core/Cursor.h"
#include "Engine/Util/Performance.h"
#include "Engine/Util/PlatformUtils.h"

#include <imgui/imgui.h>
#include <ImGuizmo/ImGuizmo.h>
#include <memory>

Engine::EditorLayer* Engine::EditorLayer::s_Instance = nullptr;

namespace Engine
{

	EditorLayer::EditorLayer()
		: Super("EditorLayer")
	{
		if (s_Instance == nullptr)
			s_Instance = this;

		m_GridMesh.m_Vertices.reserve((size_t)(m_GridLines + 1) * 3);
		m_GridMesh.m_Indices.reserve((size_t)(m_GridLines + 1) * 4);
		for (uint32 i = 0; i <= m_GridLines; i++)
		{
			float posz = (m_GridLineOffset * i) - m_GridExtent;
			m_GridMesh.m_Vertices.push_back({ { -m_GridExtent	,0 , posz }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });
			m_GridMesh.m_Vertices.push_back({ { 0				,0 , posz }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, m_GridColor.w - (m_GridColor.w * (abs(posz) / m_GridExtent)) } });
			m_GridMesh.m_Vertices.push_back({ { m_GridExtent	,0 , posz }, { m_GridColor.x, m_GridColor.y, m_GridColor.z, 0.0f } });

			m_GridMesh.m_Indices.push_back((i * 3) + 0);
			m_GridMesh.m_Indices.push_back((i * 3) + 1);
			m_GridMesh.m_Indices.push_back((i * 3) + 1);
			m_GridMesh.m_Indices.push_back((i * 3) + 2);
		}

	}

	void EditorLayer::OnAttach()
	{
		m_PlayButton = Texture2D::Create("Resources/PlayButton.png");
		m_StopButton = Texture2D::Create("Resources/StopButton.png");

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = {
			{ FrameBufferTextureFormat::RGBA8, {0.1f,0.1f,0.1f,1} },
			{ FrameBufferTextureFormat::RED_INTEGER, (Math::Vector4)-1 },
			{ FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
		};
		Window& window = Application::Get().GetWindow();
		fbSpec.Width = window.GetWidth();
		fbSpec.Height = window.GetHeight();

		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateRef<Scene>();

		m_EditorCamera = EditorCamera();

		m_HierarchyPanel.SetContext(m_ActiveScene);
		
		Renderer::SetAmbientLight({ 0.5f, 0.5f, 0.5f });
	}

	void EditorLayer::OnDetach()
	{
		
	}

	void EditorLayer::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();

		if (m_SaveScene)
		{
			SaveSceneAs();
			m_SaveScene = false;
		}

		if (m_OpenScene)
		{
			OpenScene();
			m_OpenScene = false;
		}

		// resize
		if (FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
			m_ViewPortSize.x > 0.0f && m_ViewPortSize.y > 0.0f &&
			(spec.Width != m_ViewPortSize.x || spec.Height != m_ViewPortSize.y))
		{
			m_FrameBuffer->Resize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
			m_EditorCamera.SetViewportSize(m_ViewPortSize.x, m_ViewPortSize.y);

			m_ActiveScene->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		}

		m_EditorCamera.OnUpdate();

		// setup renderer
		Renderer2D::ResetStats();

		// update scene
		if (m_SceneState == SceneState::Edit)
		{
			DrawCustomGizmo();
			m_ActiveScene->OnUpdateEditor(m_EditorCamera);
		}
		else if (m_SceneState == SceneState::Play)
			m_PlayScene->OnUpdateRuntime();

		if (Input::GetMouseButtonPressed(KeyCode::LEFT_MOUSE) && !Input::GetKeyDown(KeyCode::ALT) && !ImGuizmo::IsOver())
		{
			bool inWindow;
			int entityID = GetEntityIDAtMousePosition(inWindow);
			if (inWindow)
			{
				if (entityID == -1)
					m_HierarchyPanel.SelectEntity({});
				else
					m_HierarchyPanel.SelectEntity({ (entt::entity)entityID, m_ActiveScene.get() });
			}
		}

	}

	void EditorLayer::OnRender()
	{
		CREATE_PROFILE_FUNCTIONI();
		InstrumentationTimer timer = CREATE_PROFILEI();
		timer.Start("Recored Commands");

		Ref<CommandList> commandList = Renderer::GetMainCommandList();

		commandList->SetRenderTarget(m_FrameBuffer);
		commandList->ClearRenderTarget(m_FrameBuffer);

		m_ActiveScene->GetSceneRenderer()->Build();
		Renderer::Build();

		commandList->Present();

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

					if (ImGui::MenuItem("Open...", "Ctrl+O"))
					{
						OpenScene();

					}

					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+S"))
					{
						SaveSceneAs();
					}

					if (ImGui::MenuItem("Exit"))
						Application::Get().Close();

					ImGui::EndMenu();
				}

				ImGui::EndMenuBar();
			}

			m_HierarchyPanel.OnImGuiRender();
			m_ContentPanel.OnImGuiRender();

			
			UI_Toolbar();
			UI_Viewport();

			// Console window

			//ImGui::Begin("Console");

			//ImGui::End();

			/*ImGui::Begin("Statistics");

			ImGui::Text("Renderer2D Statis:");
			Renderer2D::Statistics stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);

			ImGui::End();*/

			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Event& e)
	{
		Super::OnEvent(e);

		m_EditorCamera.OnEvent(e);

		EventDispatcher dispacher(e);
		dispacher.Dispatch<KeyPressedEvent>(BIND_EVENT_FN(&EditorLayer::OnKeyPressed));
	}

	void EditorLayer::LoadScene(const std::string& file)
	{
		if (m_SceneState == SceneState::Play)
			OnSceneStop();

		m_LoadedScene = file;

		NewScene();
		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize(file);
	}

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{
		bool controlPressed = Input::GetKeyDown(KeyCode::CONTROL);
		bool shiftPressed = Input::GetKeyDown(KeyCode::SHIFT);
		bool rightClick = Input::GetMouseButtonDown(KeyCode::RIGHT_MOUSE);

		if (!rightClick)
		{
			switch (e.GetKeyCode())
			{
			case KeyCode::S:
			{
				if (controlPressed && shiftPressed)
					m_SaveScene = true; // TODO : fix crash and move save scene call back
				else if (controlPressed)
					SaveScene();
				break;
			}
			case KeyCode::N:
			{
				if (controlPressed)
					NewScene();
				break;
			}
			case KeyCode::O:
			{
				if (controlPressed)
					m_OpenScene = true; // TODO : fix crash and move open scene call back
				break;
			}
			}

			// imguizmo keyboard shortcuts
			switch (e.GetKeyCode())
			{
			case KeyCode::Q:
				m_GizmoType = -1;
				break;
			case KeyCode::G:
			case KeyCode::T:
				m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
				break;
			case KeyCode::S:
				m_GizmoType = ImGuizmo::OPERATION::SCALE;
				break;
			case KeyCode::R:
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
		LineRenderer::BeginScene(m_EditorCamera);
		{
			Math::Vector3 camPos = m_EditorCamera.GetPosition();

			Math::Mat4 matz =	glm::translate(Math::Mat4(1.0f), { camPos.x, 0, camPos.z - fmod(camPos.z, m_GridLineOffset) });
			Math::Mat4 matx =	glm::translate(Math::Mat4(1.0f), { camPos.x - fmod(camPos.x, m_GridLineOffset), 0, camPos.z })
								* glm::rotate(Math::Mat4(1.0f), glm::radians(90.0f), { 0,1,0 });

			LineRenderer::DrawLineMesh(m_GridMesh, matz);
			LineRenderer::DrawLineMesh(m_GridMesh, matx);
		}

		// draw camera frustum
		Entity selected = m_HierarchyPanel.GetSelectedEntity();
		if (selected && selected.HasComponent<CameraComponent>())
		{
			auto& tc = selected.GetComponent<TransformComponent>();
			auto& cam = selected.GetComponent<CameraComponent>().Camera;

			const Math::Mat4& proj = glm::inverse(cam.GetProjectionMatrix());
			const Math::Mat4& transform = tc.GetTransform();
			Math::Vector4 ltn = proj * Math::Vector4{ -1, 1, 0, 1 };
			Math::Vector4 ltf = proj * Math::Vector4{ -1, 1, 1, 1 };

			Math::Vector4 rtn = proj * Math::Vector4{ 1, 1, 0, 1 };
			Math::Vector4 rtf = proj * Math::Vector4{ 1, 1, 1, 1 };

			Math::Vector4 lbn = proj * Math::Vector4{ -1, -1, 0, 1 };
			Math::Vector4 lbf = proj * Math::Vector4{ -1, -1, 1, 1 };

			Math::Vector4 rbn = proj * Math::Vector4{ 1, -1, 0, 1 };
			Math::Vector4 rbf = proj * Math::Vector4{ 1, -1, 1, 1 };

			LineMesh mesh;
			mesh.m_Vertices = {
				{ltn / ltn.w, {1,1,1,1}}, // 0
				{rtn / rtn.w, {1,1,1,1}}, // 1
				{rbn / rbn.w, {1,1,1,1}}, // 2
				{lbn / lbn.w, {1,1,1,1}}, // 3
				{ltf / ltf.w, {1,1,1,1}}, // 4
				{rtf / rtf.w, {1,1,1,1}}, // 5
				{rbf / rbf.w, {1,1,1,1}}, // 6
				{lbf / lbf.w, {1,1,1,1}}, // 7
			};

			mesh.m_Indices = {
				0,4, 1,5, 2,6, 3,7,
				0,1, 1,2, 2,3, 3,0, // near clipping plane
				4,5, 5,6, 6,7, 7,4  // far clipping plane
			};

			LineRenderer::DrawLineMesh(mesh, transform);
		}
		LineRenderer::EndScene();
	}

	void EditorLayer::UI_Toolbar()
	{

		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, { 0,2 });
		ImGui::PushStyleVar(ImGuiStyleVar_ItemInnerSpacing, { 0,2 });
		ImGui::PushStyleColor(ImGuiCol_Button, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonHovered, { 0,0,0,0 });
		ImGui::PushStyleColor(ImGuiCol_ButtonActive, { 0,0,0,0 });

		ImGui::Begin("##toolbar", nullptr, ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse | ImGuiWindowFlags_NoTitleBar);

		float Size = ImGui::GetWindowHeight() - 4.0f;
		ImGui::SameLine((ImGui::GetWindowContentRegionMax().x*0.5f)-(Size*0.5f));
		Ref<Texture2D> button = (m_SceneState == SceneState::Edit ? m_PlayButton : m_StopButton);
		if (ImGui::ImageButton((ImTextureID)button->GetTextureHandle(), { Size, Size }, { 0,0 }, {1,1}, 0))
		{
			if (m_SceneState == SceneState::Edit)
				OnScenePlay();
			else if (m_SceneState == SceneState::Play)
				OnSceneStop();
		}

		ImGui::End();
		ImGui::PopStyleVar(2);
		ImGui::PopStyleColor(3);
	}

	void EditorLayer::UI_Viewport()
	{
		// Game window
		ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
		ImGui::Begin("Viewport##Viewport", nullptr, ImGuiWindowFlags_NoTitleBar);

		auto viewportOffset = ImGui::GetCursorPos();

		Application::Get().GetImGuiLayer()->SetBlockEvents(!ImGui::IsWindowFocused());

		ImVec2 viewPortPanalSize = ImGui::GetContentRegionAvail();
		if (m_ViewPortSize != *(Math::Vector2*)&viewPortPanalSize)
		{
			m_ViewPortSize = { viewPortPanalSize.x, viewPortPanalSize.y };
		}

		ImGui::Image((ImTextureID)m_FrameBuffer->GetAttachmentShaderHandle(0), viewPortPanalSize);
		if (ImGui::BeginDragDropTarget())
		{
			if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
			{
				bool inWindow;
				int entityID = GetEntityIDAtMousePosition(inWindow);
				if (entityID != -1)
				{
					Entity e = { (entt::entity)entityID, m_ActiveScene.get() };
					fs::path path = (const wchar_t*)payload->Data;
					if (Texture2D::ValidExtention(path.extension().string()) && e.HasComponent<SpriteRendererComponent>())
						e.GetComponent<SpriteRendererComponent>().Texture = Application::Get().GetAssetManager().GetAsset<Texture2D>(path);
				}
			}
			ImGui::EndDragDropTarget();
		}

		if (ImGui::IsItemClicked(ImGuiMouseButton_Right))
		{
			Cursor::Visibility(false);
			Cursor::Lock(true);
		}
		else if (ImGui::IsMouseReleased(ImGuiMouseButton_Right))
		{
			Cursor::Visibility(true);
			Cursor::Lock(false);
		}

		ImVec2 minBound = ImGui::GetWindowPos();
		minBound.x += viewportOffset.x;
		minBound.y += viewportOffset.y;

		ImVec2 maxBound = { minBound.x + viewPortPanalSize.x, minBound.y + viewPortPanalSize.y };
		m_ViewportBounds[0] = { minBound.x, minBound.y };
		m_ViewportBounds[1] = { maxBound.x, maxBound.y };

		Entity selected = m_HierarchyPanel.GetSelectedEntity();
		if (m_SceneState != SceneState::Play && selected)
		{
			// TODO : Fix for child entity's
			// Gizmo's
			if (m_GizmoType != -1)
			{
				// camera editor
				const Math::Mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
				Math::Mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// transform
				auto& tc = selected.GetComponent<TransformComponent>(); // get the transform component
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

	void EditorLayer::OnScenePlay()
	{
		m_SceneState = SceneState::Play;
		m_PlayScene = Scene::Copy(m_ActiveScene);
		m_HierarchyPanel.SetContext(m_PlayScene);
		m_PlayScene->OnRuntimeStart();
	}

	void EditorLayer::OnSceneStop()
	{
		m_PlayScene->OnRuntimeStop();
		m_HierarchyPanel.SetContext(m_ActiveScene);
		m_SceneState = SceneState::Edit;
	}

	int EditorLayer::GetEntityIDAtMousePosition(bool& inWindow)
	{
		inWindow = false;
		return -1; // TODO

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
			return m_FrameBuffer->ReadPixle(1, mousex, mousey);
		}

		inWindow = false;
		return -1;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = CreateRef<Scene>();
		m_ActiveScene->OnViewportResize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_HierarchyPanel.SetContext(m_ActiveScene);
	}

	void EditorLayer::OpenScene()
	{
		std::string filepath = Engine::FileDialogs::OpenFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
		{
			NewScene();
			SceneSerializer serializer(m_ActiveScene);
			serializer.Deserialize(filepath);
		}
	}

	void EditorLayer::SaveScene()
	{
		if (!m_LoadedScene.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(m_LoadedScene.string());
		}
	}

	void EditorLayer::SaveSceneAs()
	{
		std::string filepath = Engine::FileDialogs::SaveFile("Ubiq Scene (*.ubiq)\0*.ubiq\0");
		if (!filepath.empty())
		{
			SceneSerializer serializer(m_ActiveScene);
			serializer.Serialize(filepath);
		}
	}

}
