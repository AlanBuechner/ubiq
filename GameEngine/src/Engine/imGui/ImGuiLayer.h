#pragma once

#include "Engine/Core/Layer.h"
#include "Engine/Events/ApplicationEvent.h"
#include "Engine/Events/MouseEvent.h"
#include "Engine/Events/KeyEvent.h"

#if defined(PLATFORM_WINDOWS)
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "Platform/DirectX12/DX.h"
#endif

namespace Engine
{
	class Window;
	class ENGINE_API ImGuiLayer : public Layer
	{
	public:
		ImGuiLayer();
		~ImGuiLayer();

		virtual void OnAttach() override;
		virtual void OnDetach() override;
		virtual void OnImGuiRender() override;

		virtual void OnEvent(Event& e) override;

		void Begin();
		void End();
		void Build();

		void SetBlockEvents(bool block) { m_BlockEvents = block; }

	private:
		void SetDarkThemeColors();

	private:
		bool m_BlockEvents = true;
		Scope<Window> m_Window;
	};
}
