#include <Engine.h>
#include "Sandbox2D.h"
#include <Engine/Core/EntryPoint.h>

class ExampleLayer : public Engine::Layer
{
public:

	ExampleLayer()
		: Super("Example")
	{

	}

	void OnUpdate() override
	{

	}

	void OnEvent(Engine::Event& event) override
	{
		Super::OnEvent(event);
	}
};

class Sandbox : public Engine::Application
{
public:

	Sandbox() :
		Engine::Application()
	{
		PushLayer(new ExampleLayer());
		PushLayer(new Sandbox2DLayer());
	}

	~Sandbox()
	{

	}

};

Engine::Application* Engine::CreateApplication()
{
	return new Sandbox();
}