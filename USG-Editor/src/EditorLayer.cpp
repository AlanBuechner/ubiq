﻿#include "EditorLayer.h"
#include <Engine/Util/Performance.h>

#include <imgui/imgui.h>

#include <Engine/Core/Scene/SceneSerializer.h>

#include <Engine/Util/PlatformUtils.h>

#include <memory>

#include <ImGuizmo/ImGuizmo.h>

#include "Engine/Math/Math.h"


namespace Engine
{

	EditorLayer::EditorLayer()
		: Super("EditorLayer")
	{
	}

	void EditorLayer::OnAttach()
	{
		m_LogoTexture = Texture2D::Create("Assets/Images/UBIQ.png");
		m_Texture = SubTexture2D::Create(m_LogoTexture, { 2,2 }, { 0,0 }, { 2,2 });

		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { FramBufferTextureFormat::RGBA8, FramBufferTextureFormat::Depth };
		Window& window = Application::Get().GetWindow();
		fbSpec.Width = window.GetWidth();
		fbSpec.Height = window.GetHeight();

		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = std::make_shared<Scene>();


		m_EditorCamera = EditorCamera();

#if 0
		Entity entity = m_ActiveScene->CreateEntity();

		entity.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.0f, 1.0f, 0.0f, 1.0f });

		m_CameraEntity = m_ActiveScene->CreateEntity("Camera");
		m_CameraEntity.AddComponent<CameraComponent>();

		class CameraMovment : public ScriptableEntity
		{
		public:
			virtual void OnCreate() override
			{
				GetComponent<TransformComponent>();
			}

			virtual void OnDestroy() override
			{

			}

			virtual void OnUpdate() override
			{
				glm::vec3& position = GetComponent<TransformComponent>().Position;
				const float speed = 5.0f;
				float deltaTime = Time::GetDeltaTime();

				if (Input::GetKeyDown(KeyCode::A))
					position.x -= speed * deltaTime;
				if (Input::GetKeyDown(KeyCode::D))
					position.x += speed * deltaTime;
				if (Input::GetKeyDown(KeyCode::W))
					position.y += speed * deltaTime;
				if (Input::GetKeyDown(KeyCode::S))
					position.y -= speed * deltaTime;
			}
		};

		m_CameraEntity.AddComponent<NativeScriptComponent>().Bind<CameraMovment>();



		SceneSerializer serializer(m_ActiveScene);
		serializer.Deserialize("Assets/Scenes/Example.ubiq");
#endif

		m_HierarchyPanel.SetContext(m_ActiveScene);

	}

	void EditorLayer::OnDetach()
	{

	}

	void EditorLayer::OnUpdate()
	{
		CREATE_PROFILE_FUNCTIONI();

		InstrumentationTimer timer = CREATE_PROFILEI();

		// resize
		if (FrameBufferSpecification spec = m_FrameBuffer->GetSpecification();
			m_ViewPortSize.x > 0.0f && m_ViewPortSize.y > 0.0f &&
			(spec.Width != m_ViewPortSize.x || spec.Height != m_ViewPortSize.y))
		{
			m_FrameBuffer->Resize((uint32_t)m_ViewPortSize.x, (uint32_t)m_ViewPortSize.y);
			m_EditorCamera.SetViewportSize(m_ViewPortSize.x, m_ViewPortSize.y);

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewPortSize.x, (uint32_t)m_ViewPortSize.y);
		}

		m_EditorCamera.OnUpdate();

		// setup renderer
		timer.Start("Rendrer");
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		// update scene
		m_ActiveScene->OnUpdateEditor(m_EditorCamera);

		m_FrameBuffer->Unbind();

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

			// Dockspace
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
					// Disabling fullscreen would allow the window to be moved to the front of other windows, 
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

			ImGui::Begin("Statistics");


			ImGui::Text("Renderer2D Statis:");
			Renderer2D::Statistics stats = Renderer2D::GetStats();
			ImGui::Text("Draw Calls: %d", stats.DrawCalls);
			ImGui::Text("Quads: %d", stats.QuadCount);

			ImGui::End();


			// Game window
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("Viewport");
			Application::Get().GetImGuiLayer()->SetBlockEvents(!ImGui::IsWindowFocused());

			ImVec2 viewPortPanalSize = ImGui::GetContentRegionAvail();
			if (m_ViewPortSize != *(glm::vec2*)&viewPortPanalSize)
			{
				m_ViewPortSize = { viewPortPanalSize.x, viewPortPanalSize.y };
			}
			uint32_t textureID = m_FrameBuffer->GetColorAttachmentRendererID();
			ImGui::Image((void*)textureID, viewPortPanalSize, ImVec2(0, 1), ImVec2(1, 0));

			// Gizmos
			Entity selected = m_HierarchyPanel.GetSelectedEntity();
			if (selected && m_GizmoType != -1)
			{

				// camera runtime
				//auto cameraEntity = m_ActiveScene->GetPrimaryCameraEntity();
				//const auto& camera = cameraEntity.GetComponent<CameraComponent>().Camera;
				//const glm::mat4& cameraProjection = camera.GetProjectionMatrix();
				//glm::mat4 cameraView = glm::inverse(cameraEntity.GetComponent<TransformComponent>().GetTransform());

				// camera editor
				const glm::mat4& cameraProjection = m_EditorCamera.GetProjectionMatrix();
				glm::mat4 cameraView = m_EditorCamera.GetViewMatrix();

				// transform
				auto& tc = selected.GetComponent<TransformComponent>(); // get the transform component
				glm::mat4 transform = tc.GetTransform(); // get the transform matrix

				ImGuizmo::SetOrthographic(false);
				ImGuizmo::SetDrawlist();

				float windowWidth = (float)ImGui::GetWindowWidth();
				float windowHeight = (float)ImGui::GetWindowHeight();
				ImGuizmo::SetRect(ImGui::GetWindowPos().x, ImGui::GetWindowPos().y, windowWidth, windowHeight);
				
				ImGuizmo::Manipulate(	glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
										(ImGuizmo::OPERATION)m_GizmoType, ImGuizmo::LOCAL, glm::value_ptr(transform));

				if (ImGuizmo::IsUsing())
				{
					glm::vec3 position, rotation, scale;
					Math::DecomposeTransform(transform, position, rotation, scale);

					glm::vec3 deltaRotation = rotation - tc.Rotation;
					tc.Position = position;
					tc.Rotation += deltaRotation;
					tc.Scale = scale;
				}

			}

			ImGui::End();
			ImGui::PopStyleVar();


			// Console window

			ImGui::Begin("Console");

			ImGui::End();


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

	bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
	{

		bool controlPressed = Input::GetKeyDown(KeyCode::LEFT_CONTROL) || Input::GetKeyDown(KeyCode::RIGHT_CONTROL);
		bool shiftPressed = Input::GetKeyDown(KeyCode::LEFT_SHIFT) || Input::GetKeyDown(KeyCode::RIGHT_SHIFT);


		switch (e.GetKeyCode())
		{
		case KeyCode::S:
		{
			if (controlPressed && shiftPressed)
				//SaveSceneAs(); // TODO : fix crash
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
			{
				//OpenScene(); // TODO : fix crash
			}
			break;
		}

		case KeyCode::Q:
			m_GizmoType = -1;
			break;
		case KeyCode::T:
			m_GizmoType = ImGuizmo::OPERATION::TRANSLATE;
			break;
		case KeyCode::E:
			m_GizmoType = ImGuizmo::OPERATION::SCALE;
			break;
		case KeyCode::R:
			m_GizmoType = ImGuizmo::OPERATION::ROTATE;
			break;
		default:
			break;
		}

		return true;
	}

	void EditorLayer::NewScene()
	{
		m_ActiveScene = std::make_shared<Scene>();
		m_ActiveScene->OnViewportResize((uint32_t)m_ViewPortSize.x, (uint32_t)m_ViewPortSize.y);
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