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

	Engine::Timer timer = CREATE_PROFILE();

	timer.Start("std::shared_ptr");
	std::shared_ptr<Entity> e = std::make_shared<Entity>();
	timer.End();

	timer.PrintTime();

	timer.Start("Engine::SharedPtr");
	Engine::SharedPtr e2 = Engine::CreateSharedPtr<Entity>();
	timer.End();

	timer.PrintTime();


	timer.Start("Engine::UFileIO");
	Engine::UString str;
	Engine::UFileIO file;
	file.Open("Assets/Shaders/FlatColorShader.glsl");
	str = file.ReadFromFile();
	file.Close();
	timer.End();
	timer.PrintTime();
	DEBUG_INFO(str);
	DEBUG_INFO(str.Find("}"));
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
