#include "pch.h"
#include "DirectX12SwapChain.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include "DirectX12CommandQueue.h"
#include "Engine/Util/Performance.h"

namespace Engine
{

	DirectX12SwapChain::DirectX12SwapChain(Window& owningWindow) :
		m_OwningWindow(owningWindow), m_WindowHandle((HWND)owningWindow.GetNativeWindow())
	{}

	DirectX12SwapChain::~DirectX12SwapChain()
	{

	}

	void DirectX12SwapChain::Init(uint32 numBuffers)
	{
		FrameBufferSpecification fbSpec;
		fbSpec.Attachments = { {FrameBufferTextureFormat::RGBA8, {0,0,0,0}} };
		Window& window = Application::Get().GetWindow();
		fbSpec.Width = 0;
		fbSpec.Height = 0;
		fbSpec.SwapChainTarget = true;

		m_FrameBuffers.reserve(numBuffers);
		for (uint32 i = 0; i < numBuffers; i++)
			m_FrameBuffers.push_back(std::dynamic_pointer_cast<DirectX12FrameBuffer>(FrameBuffer::Create(fbSpec)));

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		wrl::ComPtr<IDXGIFactory2> factory2;
		CORE_ASSERT_HRESULT(context->GetFactory()->QueryInterface(IID_PPV_ARGS(factory2.GetAddressOf())), 
			"Failed To Get Factory2");

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		ZeroMemory(&rtvHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

		// Describe heap
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = (uint32)m_FrameBuffers.size();
		rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		rtvHeapDesc.NodeMask = NULL;

		// Create heap
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(m_DescHeap.GetAddressOf())), 
			"ID3D12Device->CreateDescriptorHeap(...) for Heap type D3D12_DESCRIPTOR_HEAP_TYPE_RTV");
		m_DescHeap->SetName(L"SwapChain RTV Heap");

		// Get heap increment size
		m_RTVHeapIncrement = context->GetDevice()->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
		CORE_ASSERT(m_RTVHeapIncrement, "Failed to get HeapHandleIncrementSize for heap type D3D12_DESCRIPTOR_HEAP_TYPE_RTV");

		// Retrieve window client rect
		RECT windowClientRect = {};
		CORE_ASSERT(GetClientRect(m_WindowHandle, &windowClientRect), "Failed to Get Cilent Rect");

		// Create Swap chain descriptor
		DXGI_SWAP_CHAIN_DESC1 swapChainDesc;
		ZeroMemory(&swapChainDesc, sizeof(DXGI_SWAP_CHAIN_DESC1));

		// Describe swap chain
		swapChainDesc.Width = windowClientRect.right - windowClientRect.left;
		swapChainDesc.Height = windowClientRect.bottom - windowClientRect.top;
		swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
		swapChainDesc.Stereo = false;		 // No 3D
		swapChainDesc.SampleDesc = { 1, 0 }; // No MSAA (not supported by D3D12 here)
		swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
		swapChainDesc.BufferCount = (uint32)m_FrameBuffers.size();
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		// Create swap chain full screen mode descriptor
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC swapChainFDesc;
		ZeroMemory(&swapChainFDesc, sizeof(DXGI_SWAP_CHAIN_FULLSCREEN_DESC));

		// Describe swap chain full screen mode
		swapChainFDesc.Windowed = !(m_OwningWindow.GetMaximised() || m_OwningWindow.GetFullScreen());

		Ref<DirectX12CommandQueue> commandQueue = Renderer::GetMainCommandQueue<DirectX12CommandQueue>();
		wrl::ComPtr<IDXGISwapChain1> swapChain;
		CORE_ASSERT_HRESULT(factory2->CreateSwapChainForHwnd(commandQueue->GetCommandQueue().Get(), m_WindowHandle, &swapChainDesc, &swapChainFDesc, nullptr, swapChain.GetAddressOf()),
			"Failed To Create Swapchain");
		CORE_ASSERT_HRESULT(swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf())), "Failed to get swap chain");

		CORE_ASSERT_HRESULT(factory2->MakeWindowAssociation(m_WindowHandle, DXGI_MWA_NO_ALT_ENTER),
			"Failed to make window association");

		GetFrameBuffers();
		m_CurrentBuffer = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void DirectX12SwapChain::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < m_FrameBuffers.size(); i++)
			m_FrameBuffers[i]->GetBuffer(0).Reset();

		// Resize swap chain
		CORE_ASSERT_HRESULT(m_SwapChain->ResizeBuffers((uint32)m_FrameBuffers.size(), width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH), 
			"Faild to Resize the swapchain");

		// Recreate buffer
		GetFrameBuffers();
		m_CurrentBuffer = m_SwapChain->GetCurrentBackBufferIndex();
	}

	void DirectX12SwapChain::SetVSync(bool val)
	{
		m_VSync = val;
	}

	void DirectX12SwapChain::SwapBuffers()
	{
		//m_SwapFlag = true;

		CREATE_PROFILE_SCOPEI("Swap Buffers");

#ifdef DEBUG
		static uint64 frame;
		OutputDebugStringA("Frame: ");
		OutputDebugStringA(std::to_string(frame).c_str());
		OutputDebugStringA("\n");
		frame++;
#endif

		CORE_ASSERT(m_SwapChain, "Invalid Swapchain! cannot swap buffers on null");

		// Present
		CORE_ASSERT_HRESULT(m_SwapChain->Present(m_VSync ? 1 : 0, m_VSync ? NULL : DXGI_PRESENT_ALLOW_TEARING), "Failed To Swap Buffers!");

	}

	void DirectX12SwapChain::UpdateBackBufferIndex()
	{
		// Increment buffer index
		m_CurrentBuffer = (m_CurrentBuffer + 1) % m_FrameBuffers.size();
	}

	void DirectX12SwapChain::CleanUp()
	{
		//SwapBuffers();
	}

	void DirectX12SwapChain::GetFrameBuffers()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		CORE_ASSERT(m_SwapChain, "Invalid Swapchain! can not get frame buffers from null");
		for (unsigned int i = 0; i < m_FrameBuffers.size(); i++) 
		{
			// Get buffer
			m_FrameBuffers[i]->GetBuffer(0).Reset();
			CORE_ASSERT_HRESULT(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(m_FrameBuffers[i]->GetBuffer(0).GetAddressOf())), "Failed to Get Frame Buffer");

			// Get heap buffer handle
			DirectX12DescriptorHandle handle;
			handle.cpu = m_DescHeap->GetCPUDescriptorHandleForHeapStart();
			handle.cpu.ptr += (size_t)m_RTVHeapIncrement * i;
			m_FrameBuffers[i]->SetDescriptorHandle(0, handle);

			// Create RTV
			context->GetDevice()->CreateRenderTargetView(m_FrameBuffers[i]->GetBuffer(0).Get(), NULL, handle.cpu);

			// Name buffer
			std::wstringstream wss;
			wss << L"Back Buffer #" << i;
			m_FrameBuffers[i]->GetBuffer(0)->SetName(wss.str().c_str());
		}
	}

}
