#include "pch.h"
#include "DirectX12Context.h"
#include "Resources/DirectX12ResourceManager.h"
#include "DX.h"

namespace Engine
{
	DirectX12Context::DirectX12Context()
	{

	}

	DirectX12Context::~DirectX12Context()
	{

	}


	void DirectX12Context::Init()
	{
#if defined(DEBUG)
		DirectX12Debug::GetInstance()->Enable();
#endif

		CORE_ASSERT_HRESULT(CreateDXGIFactory1(IID_PPV_ARGS(&m_Factory)), "Failed to create DXGIFactory");
		m_Adapter = FindAdapter();
		DXGI_ADAPTER_DESC3 desc;
		GetAdapterDesctiption(&desc);

		wrl::ComPtr<ID3D12Device> device = CreateDevice(m_MaxFeatureLevel);
		CORE_ASSERT_HRESULT(device->QueryInterface(IID_PPV_ARGS(m_Device.GetAddressOf())), "Could Not Get Device 2");

		//DXGI_QUERY_VIDEO_MEMORY_INFO memInfo;
		//CORE_ASSERT(SetAdapterVideoMemoryReservation(MEM_GiB(1)), "Failed to reserve memory");
		//GetAdapterVideoMemory(&memInfo);

		//HMONITOR monitor = MonitorFromPoint({0,0}, MONITOR_DEFAULTTOPRIMARY);
		//m_Output = GetMonitorOutput(monitor);

		//m_Format = SelectFormat();

#if defined(DEBUG)
		//DirectX12Debug::GetInstance()->ReportLiveObjects();
		DirectX12Debug::GetInstance()->InitInfoQueue(m_Device);
#endif

		m_ResoruceManager = CreateRef<DirectX12ResourceManager>();
	}

	wrl::ComPtr<IDXGIAdapter> DirectX12Context::FindAdapter(UUID preferedAdapter)
	{
		wrl::ComPtr<IDXGIAdapter> outAdapter = {};

		if (preferedAdapter != 0)
		{
			wrl::ComPtr<IDXGIFactory4> factory4;
			if (SUCCEEDED(m_Factory->QueryInterface(IID_PPV_ARGS(factory4.GetAddressOf()))))
			{
				HRESULT hr = factory4->EnumAdapterByLuid((LUID)preferedAdapter, IID_PPV_ARGS(outAdapter.GetAddressOf()));
				switch (hr)
				{
				case S_OK:
					return outAdapter;
					break;
				case DXGI_ERROR_NOT_FOUND:
					break; // use most performant adapter
				default:
					break;
				}
			}
		}

		wrl::ComPtr<IDXGIFactory6> factory6;
		if (SUCCEEDED(m_Factory->QueryInterface(IID_PPV_ARGS(factory6.GetAddressOf()))))
		{
			HRESULT hr = factory6->EnumAdapterByGpuPreference(0, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE, IID_PPV_ARGS(outAdapter.GetAddressOf()));
			switch (hr)
			{
			case S_OK:
				return outAdapter;
				break;
			case DXGI_ERROR_NOT_FOUND:
				break; // use default adapter
			default:
				break;
			}
		}

		CORE_ASSERT_HRESULT(m_Factory->EnumAdapters(0, outAdapter.GetAddressOf()), "failed to find a gpu adapter");

		return outAdapter;
	}

	wrl::ComPtr<ID3D12Device> DirectX12Context::CreateDevice(D3D_FEATURE_LEVEL& maxFeatureLevel, wrl::ComPtr<IDXGIAdapter> adapter)
	{
		if (adapter.Get() == nullptr)
			adapter = m_Adapter;

		wrl::ComPtr<ID3D12Device> device;
		CORE_ASSERT_HRESULT(D3D12CreateDevice(m_Adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf())),
			"Failed to create temp Device");

		D3D_FEATURE_LEVEL reqArray[] = {
			D3D_FEATURE_LEVEL_11_0,
			D3D_FEATURE_LEVEL_11_1,
			D3D_FEATURE_LEVEL_12_0,
			D3D_FEATURE_LEVEL_12_1,
			//D3D_FEATURE_LEVEL(0xc200), // 12_2
		};

		D3D12_FEATURE_DATA_FEATURE_LEVELS flds{0};
		flds.pFeatureLevelsRequested = reqArray;
		flds.NumFeatureLevels = _countof(reqArray);
		CORE_ASSERT_HRESULT(device->CheckFeatureSupport(D3D12_FEATURE_FEATURE_LEVELS, &flds, sizeof(D3D12_FEATURE_DATA_FEATURE_LEVELS)),
			"Failed to find feature level");

		device.Reset();
		CORE_ASSERT_HRESULT(D3D12CreateDevice(adapter.Get(), flds.MaxSupportedFeatureLevel, IID_PPV_ARGS(device.GetAddressOf())),
			"Failed to create Device");

		maxFeatureLevel = flds.MaxSupportedFeatureLevel;
		return device;
	}

	wrl::ComPtr<IDXGIOutput> DirectX12Context::GetMonitorOutput(const HMONITOR monitor, wrl::ComPtr<IDXGIAdapter> adapter)
	{
		if (adapter.Get() == nullptr)
			adapter = m_Adapter;

		wrl::ComPtr<IDXGIOutput> output;
		uint32 i = 0;
		while(adapter->EnumOutputs(i, output.GetAddressOf()) != DXGI_ERROR_NOT_FOUND) 
		{
			DXGI_OUTPUT_DESC desc{0};
			CORE_ASSERT_HRESULT(output->GetDesc(&desc), "Failed to get output descriptor");

			if (desc.Monitor == monitor)
				return output;

			output.Reset();
			i++;
		}

		//CORE_ASSERT(false, "Failed to find output");

		return wrl::ComPtr<IDXGIOutput>();
	}

	bool DirectX12Context::GetAdapterVideoMemory(DXGI_QUERY_VIDEO_MEMORY_INFO* pMem, wrl::ComPtr<IDXGIAdapter> adapter)
	{
		if (adapter.Get() == nullptr)
			adapter = m_Adapter;
		wrl::ComPtr<IDXGIAdapter3> adapter3;
		if (FAILED(adapter->QueryInterface(IID_PPV_ARGS(adapter3.GetAddressOf()))))
			return false;
		return SUCCEEDED(adapter3->QueryVideoMemoryInfo(NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, pMem));
	}

	bool DirectX12Context::SetAdapterVideoMemoryReservation(uint64 memInBytes, wrl::ComPtr<IDXGIAdapter> adapter)
	{
		if (adapter.Get() == nullptr)
			adapter = m_Adapter;
		wrl::ComPtr<IDXGIAdapter3> adapter3;
		if (FAILED(adapter->QueryInterface(IID_PPV_ARGS(adapter3.GetAddressOf()))))
			return false;
		return SUCCEEDED(adapter3->SetVideoMemoryReservation(NULL, DXGI_MEMORY_SEGMENT_GROUP_LOCAL, memInBytes));
	}

	bool DirectX12Context::GetAdapterDesctiption(DXGI_ADAPTER_DESC3* pDesc, wrl::ComPtr<IDXGIAdapter> adapter)
	{
		if (adapter.Get() == nullptr)
			adapter = m_Adapter;
		wrl::ComPtr<IDXGIAdapter4> adapter4;
		if (FAILED(adapter->QueryInterface(IID_PPV_ARGS(adapter4.GetAddressOf()))))
			return false;
		return SUCCEEDED(adapter4->GetDesc3(pDesc));
	}

	DXGI_FORMAT DirectX12Context::SelectFormat(wrl::ComPtr<IDXGIOutput> output)
	{
		if (output.Get() == nullptr)
			output = m_Output;

		if (output.Get() != nullptr)
		{
			wrl::ComPtr<IDXGIOutput1> output1;
			if (SUCCEEDED(output->QueryInterface(IID_PPV_ARGS(output1.GetAddressOf()))))
			{
				uint32 numDesc = 0;
				output1->GetDisplayModeList1(DXGI_FORMAT_UNKNOWN, 0, &numDesc, nullptr);
				DXGI_MODE_DESC1* modes = new DXGI_MODE_DESC1[numDesc];
				output1->GetDisplayModeList1(DXGI_FORMAT_UNKNOWN, 0, &numDesc, modes);

				delete[] modes;
			}
		}

		return DXGI_FORMAT_R8G8B8A8_UNORM; // default format
	}

}
