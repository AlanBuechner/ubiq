#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/Abstractions/Resources/SwapChain.h"
#include "Engine/Core/Window.h"
#include "DirectX12Texture.h"
#include "Platform/DirectX12/DX.h"

namespace Engine
{
	class RenderTarget2D;
}

namespace Engine
{
	class DirectX12SwapChain : public SwapChain
	{
	public:
		DirectX12SwapChain(Window& owningWindow);
		~DirectX12SwapChain();

		virtual void Init(uint32 numBuffers) override;
		virtual void Resize(uint32 width, uint32 height) override;

		virtual void SetVSync(bool val) override;

		virtual void SwapBuffers() override;
		virtual void UpdateBackBufferIndex() override;
		virtual void CleanUp() override;
		
		virtual uint32 GetBufferCount() override { return (uint32)m_Buffers.Count(); }

		virtual Ref<RenderTarget2D> GetCurrentRenderTarget() override { return m_Buffers[m_CurrentBuffer]; }

	private:
		void GetFrameBuffers(uint32 width, uint32 height);

	private:
		Utils::Vector<Ref<RenderTarget2D>> m_Buffers;

		wrl::ComPtr<IDXGISwapChain3> m_SwapChain;

		uint32 m_RTVHeapIncrement = 0;
		uint32 m_CurrentBuffer = 0;
		HWND m_WindowHandle;

		Window& m_OwningWindow;

		bool m_VSync;
	};
}
