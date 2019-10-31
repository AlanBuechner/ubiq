#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>


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
	Engine::UArray<int> a = {1, 2};
	Engine::UArray<int> a2(a);
	a.PushBack(3);
	a2.PushBack(3);
	if(a == a2)
		DEBUG_INFO(a2.PopBack());

	m_LogoTexture = Engine::Texture2D::Create("assets/Images/UBIQ.png");
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

	Engine::Renderer2D::DrawQuad({ 1.0f, 1.0f, 1.0f }, { 10.0f, 1.0f }, m_LogoTexture);
	Engine::Renderer2D::DrawQuad({ 1.0f, 1.0f, 0.0f }, { 1.0f, 1.0f }, m_LogoTexture);

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
