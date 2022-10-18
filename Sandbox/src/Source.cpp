#include <Engine.h>
#include "Sandbox2D.h"
#include <Engine/Core/EntryPoint.h>

class Sandbox : public Engine::Application
{
public:
	Sandbox() :
		Engine::Application()
	{
		auto timer = CREATE_PROFILEI();
		timer.Start("Sandbox2DLayer");
		PushLayer(new Sandbox2DLayer());
		m_InEditer = true;
		GenLayerStack();
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
