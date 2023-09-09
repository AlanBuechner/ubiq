#include "pch.h"
#include "DirectX12SwapChain.h"
#include "Engine/Core/Application.h"
#include "Engine/Renderer/Renderer.h"
#include "Directx12Context.h"
#include "DirectX12CommandQueue.h"
#include "Engine/Util/Performance.h"
#include "DirectX12Texture.h"

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
		m_Buffers.reserve(numBuffers);
		for (uint32 i = 0; i < numBuffers; i++)
			m_Buffers.push_back(RenderTarget2D::Create(1, 1, 1, TextureFormat::RGBA8));

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		wrl::ComPtr<IDXGIFactory2> factory2;
		CORE_ASSERT_HRESULT(context->GetFactory()->QueryInterface(IID_PPV_ARGS(factory2.GetAddressOf())),
			"Failed To Get Factory2");

		D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc;
		ZeroMemory(&rtvHeapDesc, sizeof(D3D12_DESCRIPTOR_HEAP_DESC));

		// Describe heap
		rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
		rtvHeapDesc.NumDescriptors = (uint32)m_Buffers.size();
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
		swapChainDesc.BufferCount = (uint32)m_Buffers.size();
		swapChainDesc.Scaling = DXGI_SCALING_STRETCH;
		swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
		swapChainDesc.AlphaMode = DXGI_ALPHA_MODE_IGNORE;
		swapChainDesc.Flags = DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH | DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING;

		// Create swap chain full screen mode descriptor
		DXGI_SWAP_CHAIN_FULLSCREEN_DESC* swapChainFDesc = nullptr;
		if (m_OwningWindow.GetFullScreen())
		{
			swapChainFDesc = new DXGI_SWAP_CHAIN_FULLSCREEN_DESC();

			// Describe swap chain full screen mode
			swapChainFDesc->Windowed = true;
		}

		Ref<DirectX12CommandQueue> commandQueue = Renderer::GetMainCommandQueue<DirectX12CommandQueue>();
		wrl::ComPtr<IDXGISwapChain1> swapChain;
		CORE_ASSERT_HRESULT(factory2->CreateSwapChainForHwnd(commandQueue->GetCommandQueue().Get(), m_WindowHandle, &swapChainDesc, swapChainFDesc, nullptr, swapChain.GetAddressOf()),
			"Failed To Create Swapchain");
		CORE_ASSERT_HRESULT(swapChain->QueryInterface(IID_PPV_ARGS(m_SwapChain.GetAddressOf())), "Failed to get swap chain");

		CORE_ASSERT_HRESULT(factory2->MakeWindowAssociation(m_WindowHandle, DXGI_MWA_NO_ALT_ENTER),
			"Failed to make window association");

		GetFrameBuffers();
		m_CurrentBuffer = m_SwapChain->GetCurrentBackBufferIndex();

		if (swapChainFDesc)
			delete swapChainFDesc;
	}

	void DirectX12SwapChain::Resize(uint32 width, uint32 height)
	{
		for (uint32 i = 0; i < m_Buffers.size(); i++)
		{
			DirectX12Texture2DResource* dxResource = ((DirectX12Texture2DResource*)m_Buffers[i]->GetResource());
			dxResource->m_Buffer->Release();
			dxResource->m_Buffer = 0;

		}

		// Resize swap chain
		CORE_ASSERT_HRESULT(m_SwapChain->ResizeBuffers((uint32)m_Buffers.size(), width, height, DXGI_FORMAT_UNKNOWN, DXGI_SWAP_CHAIN_FLAG_ALLOW_TEARING | DXGI_SWAP_CHAIN_FLAG_ALLOW_MODE_SWITCH),
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
		/*static uint64 frame;
		OutputDebugStringA("Frame: ");
		OutputDebugStringA(std::to_string(frame).c_str());
		OutputDebugStringA("\n");
		frame++;*/
#endif

		CORE_ASSERT(m_SwapChain, "Invalid Swapchain! cannot swap buffers on null");

		// Present
		CORE_ASSERT_HRESULT(m_SwapChain->Present(m_VSync ? 1 : 0, m_VSync ? NULL : DXGI_PRESENT_ALLOW_TEARING), "Failed To Swap Buffers!");

	}

	void DirectX12SwapChain::UpdateBackBufferIndex()
	{
		// Increment buffer index
		m_CurrentBuffer = (m_CurrentBuffer + 1) % m_Buffers.size();
	}

	void DirectX12SwapChain::CleanUp()
	{
		//SwapBuffers();
	}

	void DirectX12SwapChain::GetFrameBuffers()
	{
		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		CORE_ASSERT(m_SwapChain, "Invalid Swapchain! can not get frame buffers from null");
		for (unsigned int i = 0; i < m_Buffers.size(); i++)
		{
			// Get buffer
			DirectX12Texture2DResource* dxResource = ((DirectX12Texture2DResource*)m_Buffers[i]->GetResource());
			dxResource->m_Buffer->Release();
			dxResource->m_Buffer = 0;
			CORE_ASSERT_HRESULT(m_SwapChain->GetBuffer(i, IID_PPV_ARGS(&dxResource->m_Buffer)), "Failed to Get Frame Buffer");

			// create new handle
			m_Buffers[i]->GetSRVDescriptor()->ReBind(m_Buffers[i]->GetResource());

			// Name buffer
			std::wstringstream wss;
			wss << L"Back Buffer #" << i;
			dxResource->m_Buffer->SetName(wss.str().c_str());
		}
	}

}
