#pragma once
#include "DX.h"
#include <dxgidebug.h>

namespace Engine
{
	class DirectX12Debug
	{
	private:
		~DirectX12Debug();
	public:
		static DirectX12Debug* GetInstance();

		void InitInfoQueue(wrl::ComPtr<ID3D12Device2> device);
		void Validate();
		void Enable();
		inline bool isEnabled() { return m_IsEnabled; }
		void ReportLiveObjects();


	private:
		static DirectX12Debug* s_Instance;

		wrl::ComPtr<IDXGIDebug> m_DXGIDebug;
		wrl::ComPtr<ID3D12Debug3> m_D3DDebug;

		wrl::ComPtr<ID3D12InfoQueue> m_D3DInfoQueue;
		wrl::ComPtr<IDXGIInfoQueue> m_DXGIInfoQueue;

		bool m_IsEnabled = false;
	};
}
