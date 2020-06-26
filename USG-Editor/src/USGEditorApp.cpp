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
			m_InEditer = true; // tell the engine that we are in the editor
			m_BlockInput = true; // block input by default
			PushLayer(new EditorLayer()); // set the editor layer
			GenLayerStack();

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