#include "pch.h"
#include "DirectX12ResourceManager.h"
#include "Engine/Renderer/Renderer.h"
#include "Platform/DirectX12/Directx12Context.h"
#include "Engine/Renderer/Abstractions/GPUProfiler.h"
#include <algorithm>


Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SRVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_RTVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_DSVHeap;
Engine::Ref<Engine::DirectX12DescriptorHeap> Engine::DirectX12ResourceManager::s_SamplerHeap;

namespace Engine
{
	// resource manager
	DirectX12ResourceManager::DirectX12ResourceManager() :
		ResourceManager()
	{
		s_SRVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, true);
		s_RTVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_RTV, false);
		s_DSVHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_DSV, false);
		s_SamplerHeap = std::make_shared<DirectX12DescriptorHeap>(D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER, true);

		//m_CommandQueue = std::dynamic_pointer_cast<DirectX12CommandQueue>(CommandQueue::Create(CommandQueue::Type::Direct));
		m_BufferCopyCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(CommandList::Create(CommandList::Direct));
		m_TextureCopyCommandList = std::dynamic_pointer_cast<DirectX12CommandList>(CommandList::Create(CommandList::Direct));
	}

	DirectX12ResourceManager::~DirectX12ResourceManager()
	{
		// needs to be destroyed before the deletion pool
		for (UploadPage* page : m_UploadPages)
			delete page;

		delete m_DeletionPool; // needs to be destroyed before the heaps

		s_SRVHeap.reset();
		s_RTVHeap.reset();
		s_DSVHeap.reset();
		s_SamplerHeap.reset();
	}

}
