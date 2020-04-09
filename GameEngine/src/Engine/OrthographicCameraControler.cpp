#include "pch.h"
#include "Events/ApplicationEvent.h"
#include "Events/MouseEvent.h"
#include "OrthographicCameraControler.h"
#include "Renderer/OrthographicCamera.h"

namespace Engine
{
	OrthographicCameraControler::OrthographicCameraControler(int controlerType)
		: m_ZoomLevel(1.0f), m_ControlerType(controlerType)
	{
		Window& window = Application::Get().GetWindow();
		m_AspectRatio = (float)window.GetWidth() / (float)window.GetHeight();

		m_Camera = CreateSharedPtr<OrthographicCamera>(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	OrthographicCameraControler::OrthographicCameraControler(float aspectRatio, float zoom, int controlerType)
		: m_AspectRatio(aspectRatio), m_ZoomLevel(zoom), m_ControlerType(controlerType)
	{
		Window& window = Application::Get().GetWindow();
		window.SetViewport(window.GetHeight() * aspectRatio, window.GetHeight());
		m_Camera = CreateSharedPtr<OrthographicCamera>(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
	}

	void OrthographicCameraControler::SetPlayerInput(Ref<InputControlerManeger> maneger)
	{
		m_Input = CreateScopedPtr<InputControler>(maneger);

		if (m_ControlerType == CAMERA_CONTROLER_2D)
		{
			m_Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 0.0f,  1.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 0.0f, -1.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ -1.0f,  0.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ 1.0f,  0.0f, 0.0f })));

			m_Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 0.0f,  1.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 0.0f, -1.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ -1.0f,  0.0f, 0.0f })));
			m_Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ 1.0f,  0.0f, 0.0f })));

			// implement 2d mouse movment controles

			m_Input->BindEvent(KEYCODE_SHIFT, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Run, true));
			m_Input->BindEvent(KEYCODE_SHIFT, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Run, false));

			m_Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&OrthographicCameraControler::OnMouseMoved));
			m_Input->BindMouseMoveEvent(MOUSE_SCROLL_WHEEL, BIND_MOUSEMOVE(&OrthographicCameraControler::OnScrollWheel));
		}
		else if (m_ControlerType == CAMERA_CONTROLER_3D)
		{
			m_Input->BindEvent(KEYCODE_W, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({  0.0f, 0.0f,  1.0f })));
			m_Input->BindEvent(KEYCODE_S, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({  0.0f, 0.0f, -1.0f })));
			m_Input->BindEvent(KEYCODE_A, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({ -1.0f, 0.0f,  0.0f })));
			m_Input->BindEvent(KEYCODE_D, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Move, glm::vec3({  1.0f, 0.0f,  0.0f })));

			m_Input->BindEvent(KEYCODE_W, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({  0.0f, 0.0f,  1.0f })));
			m_Input->BindEvent(KEYCODE_S, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({  0.0f, 0.0f, -1.0f })));
			m_Input->BindEvent(KEYCODE_A, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({ -1.0f, 0.0f,  0.0f })));
			m_Input->BindEvent(KEYCODE_D, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Move, -glm::vec3({  1.0f, 0.0f,  0.0f })));

			// implement 3d mouse movment controles

			m_Input->BindEvent(KEYCODE_SHIFT, KEY_PRESSED, BIND_AXIS(&OrthographicCameraControler::Run, true));
			m_Input->BindEvent(KEYCODE_SHIFT, KEY_RELEASED, BIND_AXIS(&OrthographicCameraControler::Run, false));

			m_Input->BindMouseMoveEvent(MOUSE_DELTA, BIND_MOUSEMOVE(&OrthographicCameraControler::OnMouseMoved));
			m_Input->BindMouseMoveEvent(MOUSE_SCROLL_WHEEL, BIND_MOUSEMOVE(&OrthographicCameraControler::OnScrollWheel));
		}
	}

	void OrthographicCameraControler::OnEvent(Event& e)
	{
		EventDispatcher dispatcher(e);
		dispatcher.Dispatch<WindowResizeEvent>(BIND_EVENT_FN(&OrthographicCameraControler::OnWindowResize));
	}

	void OrthographicCameraControler::OnUpdate()
	{
		m_Camera->Translate(m_MoveDir * m_Speed * Time::GetDeltaTime());
	}

	void OrthographicCameraControler::Move(const glm::vec3& movedir)
	{
		m_MoveDir += movedir;
	}

	void OrthographicCameraControler::OnMouseMoved(glm::vec2& delta)
	{
		
	}

	void OrthographicCameraControler::OnScrollWheel(glm::vec2& delta)
	{
		float TargetZoom = m_ZoomLevel - (m_ScrollSpeed * delta.y);
		if (TargetZoom < 0.1f)
			SetZoomLevel(0.1f);
		else
			SetZoomLevel(TargetZoom);
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
}