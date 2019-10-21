#pragma once

#include "Core/core.h"
#include "Events/Event.h"
#include "Core/Input/Input.h"
#include "Core/Time.h"
#include "Engine/Renderer/Camera.h"

#define CAMERA_CONTROLER_CUSTOM(i) i+2
#define CAMERA_CONTROLER_2D 0
#define CAMERA_CONTROLER_3D 1

namespace Engine
{
	class OrthographicCamera;
	class WindowResizeEvent;
	class InputControlerManeger;
	class InputControler;
}

namespace Engine
{
	template <typename C>
	class CameraControler
	{
	public:
		virtual void SetPlayerInput(Ref<InputControlerManeger> maneger) = 0;

		virtual void OnEvent(Event& e) = 0;

		virtual void OnUpdate() = 0;

		virtual Ref<C> GetCamera() = 0;

	protected:
		virtual void SetAspectRatio(float aspectRatio) = 0;

		virtual bool OnWindowResize(WindowResizeEvent& e) = 0;

	};
}