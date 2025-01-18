#include "SceneScriptBase.h"
#include "Scene.h"
#include "Engine/Core/Application.h"

namespace Engine
{

	SceneScriptBase* SceneScriptBase::GetDefultSceneScriptInstance()
	{
		CREATE_PROFILE_FUNCTIONI();
		const std::string& sceneScriptName = Application::Get().GetProject().GetDefaultSceneScriptName();
		const Reflect::Class* scriptClass = Reflect::Registry::GetRegistry()->GetClass(sceneScriptName);
		CORE_ASSERT(scriptClass != nullptr, "Could not find defult scene script");
		return (SceneScriptBase*)scriptClass->CreateInstance();
	}

	void SceneScriptBase::SetSceneRenderer(Ref<SceneRenderer> renderer)
	{
		CREATE_PROFILE_FUNCTIONI();
		m_Scene->m_SceneRenderer = renderer;
		m_Scene->OnViewportResize(Application::Get().GetWindow().GetWidth(), Application::Get().GetWindow().GetHeight());
	}

}
	
