#include "SceneScriptBase.h"
#include "Scene.h"

namespace Engine
{

	SceneScriptBase* SceneScriptBase::GetDefultSceneScriptInstance()
	{
		const std::string& sceneScriptName = Application::Get().GetProject().GetDefaultSceneScriptName();
		const Reflect::Class* scriptClass = Reflect::Registry::GetRegistry()->GetClass(sceneScriptName);
		CORE_ASSERT(scriptClass != nullptr, "Could not find defult scene script");
		return (SceneScriptBase*)scriptClass->CreateInstance();
	}

	void SceneScriptBase::SetSceneRenderer(Ref<SceneRenderer> renderer)
	{
		m_Scene->m_SceneRenderer = renderer;
		m_Scene->OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

}
	
