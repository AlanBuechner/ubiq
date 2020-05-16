#include "Sandbox2D.h"
#include <Engine/Core/Memory/Memory.h>
#include <Engine/Util/Performance.h>
#include <Engine/Util/UFileIO.h>
#include <Engine/Util/FStream.h>
#include <imgui/imgui.h>

class Entity
{
public:
	Engine::UString name = "hello";
};

Entity* e;

Sandbox2DLayer::Sandbox2DLayer()
	: Super("Sandbox2D")
{
	m_Camera = Engine::CreateSharedPtr<Engine::OrthographicCameraControler>(1.6, 1.0f, CAMERA_CONTROLER_2D);

	m_Camera->SetPlayerInput(m_InputManeger);
}

Engine::Ref<Engine::SubTexture2D> m_Texture;

void Sandbox2DLayer::OnAttach()
{
	m_LogoTexture = Engine::Texture2D::Create("Assets/Images/UBIQ.png");
	m_Texture = Engine::SubTexture2D::Create(m_LogoTexture, { 2,2 }, { 0,0 }, { 2,2 });

	Engine::UString str;

	float i = 200.205445632521452641454f;

	str = i;

	DEBUG_INFO(i);
	DEBUG_INFO(str);

	//Engine::IFStream stream;
	//Engine::OFStream oStream;
	//stream.Open("Assets/Shaders/FlatColorShader.glsl");
	//oStream.Open("Assets/text.txt");
	//while (!stream.EndOfFile())
	//{
	//	stream.ReadLine(str);
	//	oStream.Write(str);
	//	DEBUG_INFO(str);
	//}
	//oStream.Close();
	//stream.Close();
}

void Sandbox2DLayer::OnDetach()
{

}

void Sandbox2DLayer::OnUpdate()
{
	Engine::Instrumentor::Get().RecordData(false);
	CREATE_PROFILE_FUNCTIONI();

	Engine::InstrumentationTimer timer = CREATE_PROFILEI();

	timer.Start("Camera Update");
	m_Camera->OnUpdate();
	timer.End();

	Engine::RenderCommand::SetClearColor({ 0.1f, 0.1f, 0.1f, 1 });
	Engine::RenderCommand::Clear();

	timer.Start("Rendrer");
	Engine::Renderer2D::ResetStats();
	Engine::Renderer2D::BeginScene(m_Camera->GetCamera().Get());

	Engine::Renderer2D::DrawQuad({ 0.0f, 0.0f, 0.0f }, { 1.0f, 1.0f }, m_LogoTexture);

	/*for (float x = -5.0f; x < 5.0f; x += 0.5f) 
	{
		for (float y = -5.0f; y < 5.0f; y += 0.5f)
		{
			glm::vec4 color = { (x + 5.0f) / 10.0f, 0.4f, (y + 5.0f) / 10.0f, 0.7f };
			Engine::Renderer2D::DrawQuad({ x, y, 0.0f }, { 0.45f, 0.45f }, 0.0f, color);
		}
	}*/

	Engine::Renderer2D::EndScene();

	timer.End();
	Engine::Instrumentor::Get().RecordData(false);
}

void Sandbox2DLayer::OnImGuiRender()
{
	ImGui::Begin("Statistics");
	ImGui::Text("Renderer2D Statis:");
	Engine::Renderer2D::Statistics stats = Engine::Renderer2D::GetStats();
	ImGui::Text("Draw Calls: %d", stats.DrawCalls);
	ImGui::Text("Quads: %d", stats.QuadCount);

	ImGui::End();
}

void Sandbox2DLayer::OnEvent(Engine::Event& event)
{
	Super::OnEvent(event);

	m_Camera->OnEvent(event);
}
