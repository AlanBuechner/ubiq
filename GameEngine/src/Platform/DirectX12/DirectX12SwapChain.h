#pragma once
#include "Engine/Core/Core.h"
#include "Engine/Renderer/SwapChain.h"
#include "Engine/Core/Window.h"
#include "DirectX12FrameBuffer.h"
#include "DX.h"
#include <thread>

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
		
		virtual uint32 GetBufferCount() { return (uint32)m_FrameBuffers.size(); }

		wrl::ComPtr<ID3D12DescriptorHeap> GetDescriptorHeap() { return m_DescHeap; }

		virtual Ref<FrameBuffer> GetCurrentFrameBuffer() override { return m_FrameBuffers[m_CurrentBuffer]; }

	private:
		void GetFrameBuffers();

	private:
		std::vector<Ref<DirectX12FrameBuffer>> m_FrameBuffers;
		wrl::ComPtr<IDXGISwapChain3> m_SwapChain;
		wrl::ComPtr<ID3D12DescriptorHeap> m_DescHeap;

		uint32 m_RTVHeapIncrement = 0;
		uint32 m_CurrentBuffer = 0;
		HWND m_WindowHandle;

		Window& m_OwningWindow;

		bool m_VSync;
	};
}
