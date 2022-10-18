#include "Sandbox2D.h"
#include <Engine/Util/Performance.h>
#include <imgui/imgui.h>


Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
}

void Sandbox2DLayer::OnAttach()
{
	m_LogoTexture = Engine::Texture2D::Create("Assets/Images/wood.png");
	m_Texture = Engine::SubTexture2D::Create(m_LogoTexture, { 2,2 }, { 0,0 }, { 1,2 });

	Engine::FrameBufferSpecification specs;
	specs.Attachments = {
		{ Engine::FrameBufferTextureFormat::RGBA8, {1,0,0,1} },
		{ Engine::FrameBufferTextureFormat::RED_INTEGER, (Math::Vector4)-1 },
		{ Engine::FrameBufferTextureFormat::Depth, { 1,0,0,0 } }
	};
	Engine::Window& window = Engine::Application::Get().GetWindow();
	specs.Width = window.GetWidth();
	specs.Height = window.GetHeight();
	m_Frame = Engine::FrameBuffer::Create(specs);

	m_Camera = Engine::Camera(Math::Perspective(Math::Radians(45), 16.0f/9.0f, 0.001f, 100));
	m_CameraTransform = Math::Translate({ 0,0,2.5 });

}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate()
{
	CREATE_PROFILE_FUNCTIONI();

	Engine::FrameBufferSpecification spec = m_Frame->GetSpecification();
	if (m_ViewPortSize.x > 0.0f && m_ViewPortSize.y > 0.0f &&
		(spec.Width != m_ViewPortSize.x || spec.Height != m_ViewPortSize.y))
	{
		m_Frame->Resize((uint32)m_ViewPortSize.x, (uint32)m_ViewPortSize.y);
		m_Camera = Engine::Camera(Math::Perspective(Math::Radians(45), m_ViewPortSize.x/m_ViewPortSize.y, 0.001f, 1000));
	}

}

void Sandbox2DLayer::OnRender()
{
	CREATE_PROFILE_FUNCTIONI();
	Engine::InstrumentationTimer timer = CREATE_PROFILEI();
	timer.Start("Recored Commands");

	Engine::Ref<Engine::CommandList> commandList = Engine::Renderer::GetMainCommandList();

	commandList->SetRenderTarget(m_Frame);
	commandList->ClearRenderTarget();

	//Engine::Renderer::Build();

	commandList->Present();

	timer.End();
}

void Sandbox2DLayer::OnImGuiRender()
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
	}

	// Console window

	ImGui::Begin("Statistics");

	ImGui::Text("Renderer2D Stats:");
	Engine::Renderer2D::Statistics stats = Engine::Renderer2D::GetStats();
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);
	ImGui::Text("FPS: %f", Engine::Time::GetFPS());

	ImGui::End();

	ImGui::Begin("GameWindow");

	auto viewportOffset = ImGui::GetCursorPos();

	Engine::Application::Get().GetImGuiLayer()->SetBlockEvents(!ImGui::IsWindowFocused());

	ImVec2 viewPortPanalSize = ImGui::GetContentRegionAvail();
	if (m_ViewPortSize != *(Math::Vector2*)&viewPortPanalSize)
		m_ViewPortSize = { viewPortPanalSize.x, viewPortPanalSize.y };
	ImGui::Image((ImTextureID)m_Frame->GetAttachmentShaderHandle(0), viewPortPanalSize);
	ImGui::End();

	static bool show = true;
	ImGui::ShowDemoWindow(&show);

	ImGui::End();

}

void Sandbox2DLayer::OnEvent(Engine::Event& event)
{
	Super::OnEvent(event);
}
