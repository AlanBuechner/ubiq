#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>
#include <Engine/Util/Performance.h>


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
	std::string s = "this is a string asdfaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	t1.End();

	DEBUG_INFO("std::string {0}", t1.GetMilliseconds());

	t1.Start();
	Engine::UString s2 = "this is a string asdfaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa";
	t1.End();

	DEBUG_INFO("Engine::UString {0}", t1.GetMilliseconds());
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

	Engine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 1.0f }, { 10.0f, 1.0f }, m_LogoTexture);
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
