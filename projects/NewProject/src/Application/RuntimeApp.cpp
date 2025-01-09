#include <Reflection.h>
#include <Engine.h>
#include "RuntimeLayer.h"
#include <Engine/Util/PlatformUtils.h>
#include <Engine/Core/GameBase.h>

LINK_REFLECTION_DATA(Runtime)

extern Engine::Layer* GetEditorLayer();

namespace Engine
{
	class Runtime : public Application
	{
	public:

		Runtime() :
			Application("USG Editor")
		{
			auto timer = CREATE_PROFILEI();
			timer.Start("Create Layer");

			Engine::Layer* editorLayer = GetEditorLayer();
			m_InEditer = editorLayer != nullptr;
			if(editorLayer != nullptr)
				PushLayer(editorLayer); // set the editor layer
			else
				PushLayer(new RuntimeLayer()); // set the runtime layer
			GenLayerStack();

			timer.End();
		}

		~Runtime()
		{}

	};
}

Engine::Application* CreateApplication()
{
	CREATE_PROFILE_FUNCTIONI();
	return new Engine::Runtime();
}
