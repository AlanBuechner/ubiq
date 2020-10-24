#include "EditorLayer.h"
#include <Engine/Core/Memory/Memory.h>
#include <Engine/Util/Performance.h>
#include <Engine/Util/UFileIO.h>
#include <Engine/Util/FStream.h>

#include <imgui/imgui.h>

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
		Window& window = Application::Get().GetWindow();
		fbSpec.Width = window.GetWidth();
		fbSpec.Height = window.GetHeight();

		m_FrameBuffer = FrameBuffer::Create(fbSpec);

		m_ActiveScene = CreateSharedPtr<Scene>();

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

			m_ActiveScene->OnViewportResize((uint32_t)m_ViewPortSize.x, (uint32_t)m_ViewPortSize.y);
		}

		// setup renderer
		timer.Start("Rendrer");
		Renderer2D::ResetStats();
		m_FrameBuffer->Bind();
		RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
		RenderCommand::Clear();

		// update scene
		m_ActiveScene->OnUpdate();

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

			ImGui::End();
			ImGui::PopStyleVar();


			// Console window

			ImGui::Begin("Console");

			ImGui::End();


			ImGui::End();
		}
	}

	void EditorLayer::OnEvent(Event& event)
	{
		Super::OnEvent(event);
	}
}