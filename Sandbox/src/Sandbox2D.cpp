#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>
#include <Engine/Util/Performance.h>
#include <Engine/Util/UFileIO.h>


class Entity
{
public:
	Engine::UString name = "hello";
};

Entity* e;

Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
	m_Camera = Engine::CreateSharedPtr<Engine::OrthographicCameraControler>(1.6f, 1.0f, CAMERA_CONTROLER_2D);

	m_Camera->SetPlayerInput(m_InputManeger);
}

void Sandbox2DLayer::OnAttach()
{
	Engine::FreeListAllocator* alloc = Engine::UString::s_UStringAllocator;

	alloc->StartMemoryDebuging("String Allocator", "test.json");

	m_LogoTexture = Engine::Texture2D::Create("assets/Images/UBIQ.png");

	Engine::Timer timer = CREATE_PROFILE();

	timer.Start("Engine::UFileIO");
	Engine::UString str;
	Engine::UFileIO file;
	file.Open("Assets/Shaders/FlatColorShader.glsl");
	str = file.ReadFromFile();
	file.Close();
	timer.End();

	DEBUG_INFO(str);
	DEBUG_INFO(str.Find("aaaaaaa"));
	alloc->StopMemoryDebuging();

}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate()
{
	CREATE_PROFILE_FUNCTIONI();

	Engine::InstrumentationTimer timer = CREATE_PROFILEI();

	timer.Start("Camera Update");
	m_Camera->OnUpdate();
	timer.End();

	Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Engine::RenderCommand::Clear();

	timer.Start("Rendrer");
	Engine::Renderer2D::BeginScene(*m_Camera->GetCamera());

	Engine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_LogoTexture);

	Engine::Renderer2D::EndScene();
	//Engine::Renderer::Flush();

	timer.End();
}

void Sandbox2DLayer::OnImGuiRender()
{
	
}

void Sandbox2DLayer::OnEvent(Engine::Event& event)
{
	Super::OnEvent(event);

	m_Camera->OnEvent(event);
}
