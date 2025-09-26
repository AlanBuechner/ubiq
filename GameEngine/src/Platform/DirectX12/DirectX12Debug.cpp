#include "pch.h"
#include "DirectX12Debug.h"
#include "Engine/Renderer/Renderer.h"
#include "DirectX12Context.h"

Engine::DirectX12Debug* Engine::DirectX12Debug::s_Instance = nullptr;

namespace Engine
{
	DirectX12Debug::~DirectX12Debug()
	{
		ReportLiveObjects();
	}

	DirectX12Debug* DirectX12Debug::GetInstance()
	{
		if (s_Instance == nullptr)
		{
			s_Instance = new Engine::DirectX12Debug;
			s_Instance->Validate();
		}
		return s_Instance; 
	}

	void DirectX12Debug::InitInfoQueue(wrl::ComPtr<ID3D12Device2> device)
	{
		// DirectX 12 info queue
		CORE_ASSERT_HRESULT(device->QueryInterface(IID_PPV_ARGS(m_D3DInfoQueue.GetAddressOf())),
			"Failed to get info queue");

		D3D12_MESSAGE_SEVERITY Severities[] =
		{
			D3D12_MESSAGE_SEVERITY_INFO,
			D3D12_MESSAGE_SEVERITY_WARNING,
		};

		// Suppress individual messages by their ID
		D3D12_MESSAGE_ID DenyIds[] = {
			D3D12_MESSAGE_ID_CLEARRENDERTARGETVIEW_MISMATCHINGCLEARVALUE,   // I'm really not sure how to avoid this message.
			D3D12_MESSAGE_ID_MAP_INVALID_NULLRANGE,                         // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_UNMAP_INVALID_NULLRANGE,                       // This warning occurs when using capture frame while graphics debugging.
			D3D12_MESSAGE_ID_CREATERESOURCE_STATE_IGNORED,
			D3D12_MESSAGE_ID_CREATERESOURCE_INVALIDALIGNMENT,
		};

		D3D12_INFO_QUEUE_FILTER NewFilter = {};
		NewFilter.DenyList.NumSeverities = _countof(Severities);
		NewFilter.DenyList.pSeverityList = Severities;
		NewFilter.DenyList.NumIDs = _countof(DenyIds);
		NewFilter.DenyList.pIDList = DenyIds;

		m_D3DInfoQueue->PushStorageFilter(&NewFilter);

		m_D3DInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		m_D3DInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		//m_D3DInfoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);

		// DXGI info queue
		if (SUCCEEDED(DXGIGetDebugInterface1(0, IID_PPV_ARGS(m_DXGIInfoQueue.GetAddressOf()))))
		{
			m_DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_ERROR, true);
			m_DXGIInfoQueue->SetBreakOnSeverity(DXGI_DEBUG_ALL, DXGI_INFO_QUEUE_MESSAGE_SEVERITY_CORRUPTION, true);

			DXGI_INFO_QUEUE_MESSAGE_ID hide[] =
			{
				80 /* IDXGISwapChain::GetContainingOutput: The swapchain's adapter does not control the output on which the swapchain's window resides. */,
			};
			DXGI_INFO_QUEUE_FILTER filter = {};
			filter.DenyList.NumIDs = _countof(hide);
			filter.DenyList.pIDList = hide;
			m_DXGIInfoQueue->AddStorageFilterEntries(DXGI_DEBUG_DXGI, &filter);
		}

	}

	void DirectX12Debug::Validate()
	{
		if (!m_DXGIDebug.Get())
			CORE_ASSERT_HRESULT(DXGIGetDebugInterface1(NULL, IID_PPV_ARGS(m_DXGIDebug.GetAddressOf())),
				"Failed to get DXGI Debug Interface");
		if (!m_D3DDebug.Get())
			CORE_ASSERT_HRESULT(D3D12GetDebugInterface(IID_PPV_ARGS(m_D3DDebug.GetAddressOf())),
				"Failed to get D3D12 Debug Interface");
	}

	void DirectX12Debug::Enable()
	{
		m_D3DDebug->EnableDebugLayer();
		m_D3DDebug->SetEnableGPUBasedValidation(true);
		m_IsEnabled = true;
	}

	void DirectX12Debug::ReportLiveObjects()
	{
		CORE_ASSERT_HRESULT(m_DXGIDebug->ReportLiveObjects(DXGI_DEBUG_ALL, (DXGI_DEBUG_RLO_FLAGS)(DXGI_DEBUG_RLO_IGNORE_INTERNAL | DXGI_DEBUG_RLO_DETAIL)), 
			"Failed to Report Live Objects");
	}

}

