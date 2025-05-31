#pragma once
#include "Engine/Renderer/Abstractions/Resources/ResourceManager.h"
#include "DirectX12Descriptors.h"
#include "Platform/DirectX12/DX.h"

#include "Platform/DirectX12/DirectX12CommandQueue.h"
#include "Platform/DirectX12/DirectX12CommandList.h"
#include "Engine/Renderer/Abstractions/Resources/UploadBuffer.h"

namespace Engine
{
	class DirectX12Context;
}

namespace Engine
{

	class DirectX12ResourceManager : public ResourceManager
	{

	public:
		static Ref<DirectX12DescriptorHeap> s_SRVHeap;
		static Ref<DirectX12DescriptorHeap> s_RTVHeap;
		static Ref<DirectX12DescriptorHeap> s_DSVHeap;
		static Ref<DirectX12DescriptorHeap> s_SamplerHeap;

	public:

		DirectX12ResourceManager();
		virtual ~DirectX12ResourceManager() override;

	};
}
