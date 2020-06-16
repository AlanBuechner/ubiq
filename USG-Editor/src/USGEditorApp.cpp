#include <Engine.h>
#include "EditorLayer.h"
#include <Engine/Core/EntryPoint.h>

namespace Engine
{

	class USGEditor : public Application
	{
	public:

		USGEditor() :
			Application("USG Editor")
		{
			auto timer = CREATE_PROFILEI();
			timer.Start("EditorLayer");
			PushLayer(new EditorLayer());
			timer.End();
		}

		~USGEditor()
		{

		}

	};

	Application* CreateApplication()
	{
		CREATE_PROFILE_FUNCTIONI();
		return new USGEditor();
	}
}