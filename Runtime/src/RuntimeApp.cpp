#include <Reflection.h>
#include <Engine.h>
#include "RuntimeLayer.h"
#include <Engine/Core/EntryPoint.h>
#include <Engine/Util/PlatformUtils.h>
#include <Engine/Core/GameBase.h>

LINK_REFLECTION_DATA(Runtime)

namespace Engine
{
	class Runtime : public Application
	{
	public:

		Runtime() :
			Application("USG Editor")
		{
			auto timer = CREATE_PROFILEI();
			timer.Start("Runtime Layer");
			PushLayer(new RuntimeLayer()); // set the editor layer
			GenLayerStack();

			timer.End();
		}

		~Runtime()
		{

		}

	};

	Application* CreateApplication()
	{
		CREATE_PROFILE_FUNCTIONI();
		return new Runtime();
	}
}
