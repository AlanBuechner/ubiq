#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>
#include <Engine/Util/UString.h>

class Entity
{
public:
	Engine::UString name = "hello";
};

Entity* e;

Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
	m_Camera.reset(new Engine::OrthographicCameraControler(1.6f, 1.0f, CAMERA_CONTROLER_2D));

	m_Camera->SetPlayerInput(m_InputManeger);
}

void Sandbox2DLayer::OnAttach()
{
	e = Engine::CreateObject<Entity>();
	Engine::UString str = "this is a string this is alot longer then the other string";
	Engine::UString otherStr("this is another string ");
	Engine::UString str3("yet another string");
	Engine::UString str4("yet another another string");
	str3 += str4;
	str4 = "this is rather difrent";
	{
		Engine::UString str5;
		DEBUG_INFO(str5);
	}
	Engine::UString str6("yet another string");
	DEBUG_INFO(str);
	DEBUG_INFO(otherStr);
	DEBUG_INFO(str3);
	DEBUG_INFO(str4);
	DEBUG_INFO(str6 + str4);
	DEBUG_INFO(e->name + " hello world");
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

	Engine::Renderer2D::DrawQuad({ 1.0f, 1.0f }, { 1.0f, 1.0f }, { 0.8f, 0.2f, 0.3f, 1.0f });

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
