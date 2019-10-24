#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>

class Entity
{
public:
	Entity()
	{
		DEBUG_INFO("created new Entity {0}, {1}", a, c);
		name = "hello";
	}

	int a = 6;
	float c = 4.5f;
	std::string name;
};

Entity* entity;
Entity* entity2;

Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
	m_Camera.reset(new Engine::OrthographicCameraControler(1.6f, 1.0f, CAMERA_CONTROLER_2D));

	m_Camera->SetPlayerInput(m_InputManeger);
}

void Sandbox2DLayer::OnAttach()
{
	entity = Engine::CreateObject<Entity>();
	entity2 = Engine::CreateObject<Entity>();
	entity2->name = "this is another name";
	DEBUG_INFO("{0}", entity->name);
	DEBUG_INFO("{0}", entity2->name);
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
