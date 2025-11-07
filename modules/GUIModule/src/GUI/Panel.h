#pragma once
#include "Engine/Math/Math.h"
#include "Engine/Events/Event.h"

namespace GUI
{
	class Context;
	class SplitPanel;
}

namespace GUI
{
	enum SplitAxis
	{
		X, Y
	};

	struct PanelData
	{
		Math::Vector4 color = { 0,0,0,0 };
		Math::Vector4 borderColor = { 0,0,0,0 };
		float borderWidth = 0;
		float borderHardness = 0;
	};

	struct PanelTransform
	{
		Math::Vector2 position = { 0,0 };
		Math::Vector2 size = { 0,0 };
	};

	class PanelBase
	{
	public:

		virtual ~PanelBase();

		virtual void Init() {};
		virtual void OnDestroy() {};

		virtual void OnGUIRender();

		void Split(PanelBase* other, SplitAxis axis);
		void ClearParent();
		void SetFloating(bool val);
		bool IsFloating() { return m_Floating; }
		bool IsContentPanel() { return m_ContentPanel; }

		Context* GetContext() { return m_Context; }

		virtual void AttachChild(PanelBase* chiled) { CORE_ASSERT(false, "panels do not support children", ""); };
		virtual void DetatchChiled(PanelBase* chiled) { CORE_ASSERT(false, "panels do not support children", ""); };

	protected:
		virtual PanelTransform GetChildBounds(PanelBase* panel) {};

	private:
		virtual bool OnEvent(Engine::Event* e);

	protected:
		bool m_DrawPanel = true;
		PanelTransform m_Transform;
		PanelData m_DrawData;

	private:
		bool m_ContentPanel = true;
		bool m_Floating = false;
		Context* m_Context = nullptr;
		PanelBase* m_Parent = nullptr;

		friend class Context;
		friend class SplitPanel;
	};



	class SplitPanel : public PanelBase
	{
	public:
		SplitPanel(SplitAxis axis, float splitAmount);

		virtual ~SplitPanel() override;

		virtual void OnDestroy() override;

		virtual void Init() override;

		virtual void OnGUIRender() override;

		virtual void AttachChild(PanelBase* chiled) override;
		virtual void DetatchChiled(PanelBase* chiled) override;

		virtual PanelTransform GetChildBounds(PanelBase* panel) override;

	private:

		virtual bool OnEvent(Engine::Event* e) override;

		Math::Vector2 GetSplitPoint();
		void SetSplitPoint(Math::Vector2 point);

	private:
		SplitAxis m_Axis = X;
		float m_SplitPercent = 0.5f;

		PanelBase* m_Child1 = nullptr;
		PanelBase* m_Child2 = nullptr;
	};


}
