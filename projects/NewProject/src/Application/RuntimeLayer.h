#pragma once
#include <Engine.h>
#include <Engine/Math/Math.h>

#include "Engine/Renderer/EditorCamera.h"
#include "Engine/Renderer/SceneRenderer.h"

#include "ProjectManager/Project.h"
#include "Engine/Core/GameBase.h"

namespace Engine
{
	class RuntimeLayer : public Layer
	{
	public:
		RuntimeLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;

		virtual void OnUpdate() override;
		virtual void OnRender() override;
		virtual void OnImGuiRender() override;
		virtual void OnEvent(Event* event) override;

	private:

		GameBase* m_Game;
	};
}
