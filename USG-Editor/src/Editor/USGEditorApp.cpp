#include <Reflection.h>
#include <Engine.h>
#include "EditorLayer.h"
#include "EditorAssets.h"
#include <Engine/Core/EntryPoint.h>
#include <Engine/Util/PlatformUtils.h>
#include <Engine/Core/GameBase.h>

LINK_REFLECTION_DATA(USG_Editor)

namespace Editor
{
	class USGEditor : public Engine::Application
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

}

Engine::Application* CreateApplication()
{
	CREATE_PROFILE_FUNCTIONI();
	return new Editor::USGEditor();
}
