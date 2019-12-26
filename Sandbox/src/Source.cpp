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
		auto timer = CREATE_PROFILEI();
		timer.Start("ExampleLayer");
		PushLayer(new ExampleLayer());
		timer.End();
		timer.Start("Sandbox2DLayer");
		PushLayer(new Sandbox2DLayer());
		timer.End();
	}

	~Sandbox()
	{

	}

};

Engine::Application* Engine::CreateApplication()
{
	CREATE_PROFILE_FUNCTIONI();
	return new Sandbox();
}