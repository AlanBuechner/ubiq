#include <Engine.h>
#include "EditorLayer.h"
#include "EditorAssets.h"
#include <Engine/Core/EntryPoint.h>
#include <Engine/Util/PlatformUtils.h>

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
			EditorAssets::Init();
			PushLayer(new EditorLayer()); // set the editor layer
			GenLayerStack();

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
