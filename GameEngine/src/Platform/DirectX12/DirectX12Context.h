#pragma once
#include "Resources/DirectX12SwapChain.h"
#include "Engine/Renderer/Abstractions/GraphicsContext.h"
#include "Engine/Core/UUID.h"
#include "DX.h"
#include "DirectX12Debug.h"
#include "Resources/DirectX12ResourceManager.h"

namespace Engine
{
	class DirectX12Context : public GraphicsContext
	{
	public:
		DirectX12Context();
		~DirectX12Context();

		virtual void Init() override;

		virtual Ref<ResourceManager> GetResourceManager() override { return m_ResoruceManager; }
		Ref<DirectX12ResourceManager> GetDX12ResourceManager() { return m_ResoruceManager; }

		inline wrl::ComPtr<IDXGIFactory1> GetFactory() { return m_Factory; }
		inline wrl::ComPtr<IDXGIAdapter> GetAdapter() { return m_Adapter; }
		inline wrl::ComPtr<ID3D12Device14> GetDevice() { return m_Device; }
		inline wrl::ComPtr<IDXGIOutput> GetOutput() { return m_Output; }
		inline DXGI_FORMAT GetFormat() { return m_Format; };

		wrl::ComPtr<IDXGIAdapter> FindAdapter(UUID preferedAdapter = 0);
		wrl::ComPtr<ID3D12Device> CreateDevice(D3D_FEATURE_LEVEL& maxFeatureLevel, wrl::ComPtr<IDXGIAdapter> adapter = nullptr);
		wrl::ComPtr<IDXGIOutput> GetMonitorOutput(const HMONITOR monitor, wrl::ComPtr<IDXGIAdapter> adapter = nullptr);
		
		bool GetAdapterVideoMemory(DXGI_QUERY_VIDEO_MEMORY_INFO* pMem, wrl::ComPtr<IDXGIAdapter> adapter = nullptr);
		bool SetAdapterVideoMemoryReservation(uint64 memInBytes, wrl::ComPtr<IDXGIAdapter> adapter = nullptr);

		bool GetAdapterDesctiption(DXGI_ADAPTER_DESC3* pDesc, wrl::ComPtr<IDXGIAdapter> adapter = nullptr);

		DXGI_FORMAT SelectFormat(wrl::ComPtr<IDXGIOutput> output = nullptr);

	private:
		wrl::ComPtr<IDXGIFactory1> m_Factory;
		wrl::ComPtr<IDXGIAdapter> m_Adapter;
		wrl::ComPtr<ID3D12Device14> m_Device;
		wrl::ComPtr<IDXGIOutput> m_Output;
		D3D_FEATURE_LEVEL m_MaxFeatureLevel = D3D_FEATURE_LEVEL_11_0;
		DXGI_FORMAT m_Format = DXGI_FORMAT_R8G8B8A8_UNORM;

		Ref<DirectX12ResourceManager> m_ResoruceManager;
	};
}
