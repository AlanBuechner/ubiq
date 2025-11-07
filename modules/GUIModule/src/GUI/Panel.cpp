#include "Panel.h"
#include "GUI.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Core/Input/KeyCodes.h"
#include "Engine/Core/Input/Input.h"
#include <cmath>
#include <math.h>

namespace GUI
{


	// ------------------------------------------- Panel Base ------------------------------------------- //
	PanelBase::~PanelBase()
	{
		ClearParent();
	}
	
	void PanelBase::OnGUIRender()
	{
		// update dimensions
		if (m_Floating == false)
		{
			m_Transform.position = { 0,0 };
			m_Transform.size = { m_Context->GetAspect(), 1 };

			if (m_Parent != nullptr)
				m_Transform = m_Parent->GetChildBounds(this);
		}

		// add panel draw
		if (m_DrawPanel)
		{
			m_Context->Panel(m_Transform.position, m_Transform.size, m_DrawData);
			m_Context->PushDepth();
		}
	}

	void PanelBase::Split(PanelBase* other, SplitAxis axis)
	{
		PanelBase* parent = m_Parent;

		SplitPanel* split = m_Context->CreatePanel<SplitPanel>(axis, 0.5f).GetAs<SplitPanel>();
		if (parent)
		{
			parent->DetatchChiled(this);
			parent->AttachChild(split);
		}

		split->AttachChild(this);
		split->AttachChild(other);
	}

	void PanelBase::ClearParent()
	{
		PanelBase* parent = m_Parent; // this is to prevent infinite recursion
		m_Parent = nullptr;
		if (parent)
			parent->DetatchChiled(this);
	}

	void PanelBase::SetFloating(bool val)
	{
		m_Floating = val;

		if (m_Floating)
			ClearParent();
	}

	bool PanelBase::OnEvent(Engine::Event* e)
	{
		if (m_Context->GetInputLock() != nullptr) return false; // dont call parent on event if a panel is locking input
		if (m_Parent && m_Parent->OnEvent(e)) return true;

		return false;
	}











	// ------------------------------------------- Split Panel ------------------------------------------- //
	SplitPanel::SplitPanel(SplitAxis axis, float splitAmount)
	{
		m_ContentPanel = false;
		m_Axis = axis;
		m_SplitPercent = splitAmount;
	}

	SplitPanel::~SplitPanel()
	{
		DetatchChiled(m_Child1);
		DetatchChiled(m_Child2);
	}

	void SplitPanel::OnDestroy()
	{
		delete m_Child1; // when the child is deleted 
		delete m_Child2;
	}

	void SplitPanel::Init()
	{
		m_DrawPanel = false;
	}

	void SplitPanel::OnGUIRender()
	{
		PanelBase::OnGUIRender();

		if (m_Child1 != nullptr) m_Child1->OnGUIRender();
		if (m_Child2 != nullptr) m_Child2->OnGUIRender();
	}

	void SplitPanel::AttachChild(PanelBase* chiled)
	{
		if (m_Child1 == nullptr) m_Child1 = chiled;
		else if (m_Child2 == nullptr) m_Child2 = chiled;
		else return;

		if (chiled->m_Parent) chiled->m_Parent->ClearParent();

		chiled->m_Parent = this;
	}

	void SplitPanel::DetatchChiled(PanelBase* chiled)
	{
		if (chiled == nullptr) return;

		if (chiled == m_Child1)
			m_Child1 = nullptr;
		else if (chiled == m_Child2)
			m_Child2 = nullptr;
		else return;

		// only clear the parent if it was a child panel
		chiled->ClearParent();
	}

	PanelTransform SplitPanel::GetChildBounds(PanelBase* panel)
	{
		if (panel != m_Child1 && panel != m_Child2) return PanelTransform(); // not a child panel

		Math::Vector2 splitSize = m_Transform.size;
		if (m_Axis == SplitAxis::X) splitSize.x = std::lerp(0.0f, m_Transform.size.x, m_SplitPercent);
		if (m_Axis == SplitAxis::Y) splitSize.y = std::lerp(0.0f, m_Transform.size.y, m_SplitPercent);

		PanelTransform out;
		if (panel == m_Child1)
		{
			out.position = m_Transform.position;
			out.size = splitSize;

		}
		else if (panel == m_Child2)
		{
			Math::Vector2 splitPoint = GetSplitPoint();

			out.position = splitPoint;
			out.size = m_Transform.size - splitSize;
			if (m_Axis == SplitAxis::X) out.size.y = m_Transform.size.y;
			if (m_Axis == SplitAxis::Y) out.size.x = m_Transform.size.x;
		}

		return out;
	}

	bool SplitPanel::OnEvent(Engine::Event* e)
	{
		if (PanelBase::OnEvent(e)) return true;

		// get current mouse position
		Math::Vector2 mousePos = Engine::Input::GetMousePosition();
		m_Context->ConvertToGUICoords(mousePos); // convert to GUI cords

		Math::Vector2 splitPoint = GetSplitPoint();

		// check if on bounds
		bool hoveringSplit = false;
		if (m_Axis == SplitAxis::X) hoveringSplit = Math::InError(mousePos.x, splitPoint.x, 0.004f);
		if (m_Axis == SplitAxis::Y) hoveringSplit = Math::InError(mousePos.y, splitPoint.y, 0.004f);

		if (hoveringSplit && e->GetEventType() == Engine::EventType::MouseButtonPressed) // check for start dragging
		{
			Engine::MouseCode mouseButton = ((Engine::MouseButtonPressedEvent*)e)->GetMouseButton();
			if (mouseButton == Engine::MouseCode::LEFT_MOUSE)
				m_Context->SetInputLock(this);
		}

		bool moveingSplit = m_Context->GetInputLock() == this;
		if (moveingSplit && e->GetEventType() == Engine::EventType::MouseButtonReleased) // check for end dragging
		{
			Engine::MouseCode mouseButton = ((Engine::MouseButtonReleasedEvent*)e)->GetMouseButton();
			if (mouseButton == Engine::MouseCode::LEFT_MOUSE)
				m_Context->ClearInputLock();
		}

		if (moveingSplit)
			SetSplitPoint(mousePos);

		return moveingSplit || hoveringSplit;
	}

	Math::Vector2 SplitPanel::GetSplitPoint()
	{
		Math::Vector2 splitPoint = m_Transform.position;
		if (m_Axis == SplitAxis::X) splitPoint.x = std::lerp(m_Transform.position.x, m_Transform.position.x + m_Transform.size.x, m_SplitPercent);
		if (m_Axis == SplitAxis::Y) splitPoint.y = std::lerp(m_Transform.position.y, m_Transform.position.y + m_Transform.size.y, m_SplitPercent);

		return splitPoint;
	}

	void SplitPanel::SetSplitPoint(Math::Vector2 point)
	{
		Math::Vector2 localPoint = point - m_Transform.position;
		if (m_Axis == SplitAxis::X) m_SplitPercent = localPoint.x / m_Transform.size.x;
		if (m_Axis == SplitAxis::Y) m_SplitPercent = localPoint.y / m_Transform.size.y;
	}

}

