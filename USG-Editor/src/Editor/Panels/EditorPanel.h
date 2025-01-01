#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Core/Scene/Scene.h"

#include <Reflection.h>

namespace Editor
{
	class EditorPanel : public Reflect::Reflected
	{
	public:
		virtual void OnUpdate() {}
		virtual void OnImGuiRender() {}
		virtual void OnSceneChange(Engine::Ref<Engine::Scene> scene) {}
		virtual void OnScreenClick(Math::Vector2 pos) {}

		virtual void OnEvent(Engine::Event* e) {}

		virtual void OnDrawGizmos() {}
	};
}
