#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/SwapChain.h"
#include "Engine/Core/Window.h"
#include "DirectX12Texture.h"
#include "DX.h"

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
		
		virtual uint32 GetBufferCount() override { return (uint32)m_Buffers.size(); }

		wrl::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_DescHeap; }

		virtual Ref<RenderTarget2D> GetCurrentRenderTarget() override { return m_Buffers[m_CurrentBuffer]; }

	private:
		void GetFrameBuffers();

	private:
		std::vector<Ref<RenderTarget2D>> m_Buffers;

		wrl::ComPtr<IDXGISwapChain3> m_SwapChain;
		wrl::ComPtr<ID3D12DescriptorHeap> m_DescHeap;

		uint32 m_RTVHeapIncrement = 0;
		uint32 m_CurrentBuffer = 0;
		HWND m_WindowHandle;

		Window& m_OwningWindow;

		bool m_VSync;
	};
}
