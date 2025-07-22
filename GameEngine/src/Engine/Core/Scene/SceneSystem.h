#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class Scene;
}

namespace Engine
{
	class SceneSystem : public Reflect::Reflected
	{
	protected:
		virtual ~SceneSystem() {};

		virtual void OnAttatch() {}
		virtual void OnDetatch() {}
		virtual void OnUpdate() {}

	protected:
		Scene* m_Scene;

		friend Scene;
	};
}
