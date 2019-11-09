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
	m_LogoTexture = Engine::Texture2D::Create("assets/Images/UBIQ.png");

	Engine::Timer t1;

	t1.Start();
	std::shared_ptr<Entity> e = std::make_shared<Entity>();
	t1.End();

	DEBUG_INFO("std::shared_ptr {0}", t1.GetMicroseconds());

	t1.Start();
	Engine::SharedPtr e2 = Engine::CreateSharedPtr<Entity>();
	t1.End();

	DEBUG_INFO("Engine::SharedPtr {0}", t1.GetMicroseconds());

	t1.Start();
	Engine::UFileIO file;
	file.Open("Assets/Shaders/FlatColorShader.glsl");
	Engine::Ref<Engine::UString> str = file.ReadFromFile();
	file.Close();
	t1.End();

	DEBUG_INFO("Engine::UFileIO {0}", t1.GetMicroseconds());

}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate()
{
	m_Camera->OnUpdate();

	Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Engine::RenderCommand::Clear();

	Engine::Renderer2D::BeginScene(*m_Camera->GetCamera());

	Engine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 10.0f, 10.0f }, m_LogoTexture);
	Engine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_LogoTexture);

	Engine::Renderer2D::EndScene();
	//Engine::Renderer::Flush();
}

void Sandbox2DLayer::OnImGuiRender()
{
	
}

void Sandbox2DLayer::OnEvent(Engine::Event& event)
{
	Super::OnEvent(event);

	m_Camera->OnEvent(event);
}
