#include "pch.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "OrthographicCameraControler.h"
#include "Renderer/OrthographicCamera.h"

namespace Engine
{
	OrthographicCameraControler::OrthographicCameraControler(float aspectRatio, float zoom)
		: m_AspectRatio(aspectRatio), m_ZoomLevel(zoom)
	{
		m_Camera.reset(new OrthographicCamera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel));
	}

	void OrthographicCameraControler::SetPlayerInput(Ref<InputControlerManeger> maneger)
	{
		m_Input.reset(new InputControler(maneger));

		m_Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 0.0f,  1.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 0.0f, -1.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ -1.0f,  0.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 1.0f,  0.0f, 0.0f })));

		m_Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 0.0f,  1.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 0.0f, -1.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ -1.0f,  0.0f, 0.0f })));
		m_Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 1.0f,  0.0f, 0.0f })));

		m_Input->BindEvent(KEYCODE_SHIFT, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Run, true));
		m_Input->BindEvent(KEYCODE_SHIFT, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Run, false));

		m_Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&OrthographicCameraControler::MouseMoved));
	}

	void OrthographicCameraControler::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(&OrthographicCameraControler::OnWindowResize));
		dispatcher.Dispatch<MouseScrolledEvent>(BIND_EVENT_FN(&OrthographicCameraControler::OnScrollWheel));
	}

	void OrthographicCameraControler::OnUpdate()
	{
		m_Camera->Translate(m_MoveDir * m_Speed * Time::GetDeltaTime());
	}

	void OrthographicCameraControler::Move(const glm::vec3& movedir)
	{
		m_MoveDir += movedir;
	}

	void OrthographicCameraControler::MouseMoved(glm::vec2& pos)
	{
		
	}

	void OrthographicCameraControler::Run(bool run)
	{
		m_Speed = run ? m_RunSpeed : m_WalkSpeed;
	}

	void OrthographicCameraControler::SetZoomLevel(float zoom)
	{
		m_ZoomLevel = zoom;
		m_Camera->SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthographicCameraControler::SetAspectRatio(float aspectRatio)
	{
		m_AspectRatio = aspectRatio;
		m_Camera->SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	bool OrthographicCameraControler::OnWindowResize(WindowResizeEvent& e)
	{
		SetAspectRatio((float)e.GetWidth() / (float)e.GetHeight());
		return false;
	}

	bool OrthographicCameraControler::OnScrollWheel(MouseScrolledEvent& e)
	{
		float TargetZoom = m_ZoomLevel - e.GetYOffset();
		if (TargetZoom < 0.1f)
			SetZoomLevel(0.1f);
		else
			SetZoomLevel(m_ZoomLevel - e.GetYOffset());
		return false;
	}

}