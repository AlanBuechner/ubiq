#include "SceneScriptBase.h"
#include "Scene.h"

namespace Engine
{

	Ref<SceneScriptBase> SceneScriptBase::GetDefultSceneScriptInstance()
	{
		
	}

	void SceneScriptBase::SetSceneRenderer(Ref<SceneRenderer> renderer)
	{
		m_Scene->m_SceneRenderer = renderer;
		m_Scene->OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

}

