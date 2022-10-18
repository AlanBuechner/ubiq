#include <Engine.h>
#include "EditorLayer.h"
#include "Panels/EditorAssets.h"
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
			m_InEditer = true;
			PushLayer(new EditorLayer()); // set the editor layer
			GenLayerStack();
			EditorAssets::Init();

			timer.End();
		}

		~USGEditor()
		{
			EditorAssets::Destroy();
		}

	};

	Application* CreateApplication()
	{
		CREATE_PROFILE_FUNCTIONI();
		return new USGEditor();
	}
}
