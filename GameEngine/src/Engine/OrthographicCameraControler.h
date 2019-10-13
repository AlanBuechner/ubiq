#pragma once
#include "Core/Time.h"
#include "Core/Input/InputControler.h"
#include "Core/Input/Input.h"
#include <glm/glm.hpp>

namespace Engine
{
	class OrthographicCamera;
	class MouseScrolledEvent;
}

namespace Engine
{
	class OrthographicCameraControler
	{
	public:
		OrthographicCameraControler(float aspectRatio, float zoom);

		void SetPlayerInput(Ref<InputControlerManeger> maneger);

		void OnEvent(Event& e);

		void OnUpdate();

		inline Ref<OrthographicCamera> GetCamera() { return m_Camera; }
	private:
		void Move(const glm::vec3& movedir);
		void Run(bool run);
		void OnMouseMoved(glm::vec2& delta);
		void OnScrollWheel(glm::vec2& delta);

		void SetZoomLevel(float zoom);
		void SetAspectRatio(float aspectRatio);

		bool OnWindowResize(WindowResizeEvent& e);

	private:
		float m_AspectRatio;
		float m_ZoomLevel;
		Ref<OrthographicCamera> m_Camera;

		Scope<InputControler> m_Input;
		float m_WalkSpeed = 1.0f;
		float m_RunSpeed = 2.0f;
		float m_Speed = 1.0f;
		glm::vec3 m_MoveDir = { 0.0f, 0.0f, 0.0f };

		float m_ScrollSpeed = 0.5f;
	};
}