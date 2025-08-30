#include <Reflection.h>
#include <Engine.h>
#include <Engine/Core/EntryPoint.h>
#include "RuntimeLayer.h"
#include <Engine/PlatformUtils/PlatformUtils.h>
#include <Engine/Core/GameBase.h>

extern Engine::Layer* GetEditorLayer();

namespace Game
{
	class Runtime : public Engine::Application
	{
	public:

		Runtime() :
			Application("USG Editor")
		{
			auto timer = CREATE_PROFILEI();
			START_PROFILEI(timer, "Create Layer");

			Engine::Layer* editorLayer = GetEditorLayer();
			m_InEditer = editorLayer != nullptr;
			if(editorLayer != nullptr)
				PushLayer(editorLayer); // set the editor layer
			else
				PushLayer(new RuntimeLayer()); // set the runtime layer
			GenLayerStack();

			END_PROFILEI(timer);
		}

		~Runtime()
		{}

	};
}

Engine::Application* CreateApplication()
{
	CREATE_PROFILE_FUNCTIONI();
	return new Game::Runtime();
}
