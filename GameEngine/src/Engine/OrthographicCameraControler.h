#pragma once
#include "Core/Time.h"
#include "CameraControler.h"
#include <glm/glm.hpp>

namespace Engine
{
	class OrthographicCamera;
	class MouseScrolledEvent;
}

namespace Engine
{
	class OrthographicCameraControler : public CameraControler<OrthographicCamera>
	{
	public:
		OrthographicCameraControler(int controleType);
		OrthographicCameraControler(float aspectRatio, float zoom, int controleType);

		virtual void SetPlayerInput(Ref<InputControlerManeger> maneger) override;

		virtual void OnEvent(Event& e) override;

		virtual void OnUpdate() override;

		virtual inline Ref<OrthographicCamera> GetCamera() override { return m_Camera; }
	protected:
		void Move(const glm::vec3& movedir);
		void Run(bool run);
		void OnMouseMoved(glm::vec2& delta);
		void OnScrollWheel(glm::vec2& delta);

		void SetZoomLevel(float zoom);
		virtual void SetAspectRatio(float aspectRatio) override;

		virtual bool OnWindowResize(WindowResizeEvent& e) override;

	protected:
		float m_AspectRatio;
		float m_ZoomLevel;
		Ref<OrthographicCamera> m_Camera;

		Scope<InputControler> m_Input;
		float m_WalkSpeed = 1.0f;
		float m_RunSpeed = 2.0f;
		float m_Speed = 1.0f;
		glm::vec3 m_MoveDir = { 0.0f, 0.0f, 0.0f };

		float m_ScrollSpeed = 0.5f;

		int m_ControlerType = 0;

		typedef OrthographicCameraControler Super;
		
	};
}