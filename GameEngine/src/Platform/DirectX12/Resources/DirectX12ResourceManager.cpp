#include "pch.h"
#include "DirectX12ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/DirectX12Context.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include <algorithm>


Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SRVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_RTVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_DSVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SamplerHeap;

namespace Engine
{

	DirectX12TransientResourceHeap::DirectX12TransientResourceHeap(uint32 size)
	{
		CREATE_PROFILE_FUNCTIONI();

		m_Size = size;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();

		CD3DX12_HEAP_DESC heapDesc(m_Size, D3D12_HEAP_TYPE_DEFAULT, 0, D3D12_HEAP_FLAG_NONE);
		HRESULT hr = context->GetDevice()->CreateHeap(&heapDesc, IID_PPV_ARGS(&m_Heap));

		CORE_ASSERT_HRESULT(hr, "Failed to create transient heap");
	}

	DirectX12TransientResourceHeap::~DirectX12TransientResourceHeap()
	{
		m_Heap->Release();
		m_Heap = nullptr;
	}






	// resource manager
	DirectX12ResourceManager::DirectX12ResourceManager() :
		ResourceManager()
	{
		s_SRVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
		s_RTVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
		s_DSVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
		s_SamplerHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);
	}

	DirectX12ResourceManager::~DirectX12ResourceManager()
	{
		s_SRVHeap.reset();
		s_RTVHeap.reset();
		s_DSVHeap.reset();
		s_SamplerHeap.reset();
	}

}
