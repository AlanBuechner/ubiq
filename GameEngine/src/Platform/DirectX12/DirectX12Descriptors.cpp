#include "pch.h"
#include "DirectX12Descriptors.h"
#include "Directx12Context.h"
#include "Engine/Renderer/Renderer.h"

namespace Engine
{

	void DirectX12DescriptorHandle::Free()
	{
		if (m_Container)
			m_Container->Free(*this);
	}

	DirectX12DescriptorHeap::DirectX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisable) :
		m_Type(type), m_IsShaderVisable(isShaderVisable)
	{
		if (m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_DSV || m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
			m_IsShaderVisable = false;

		Ref<DirectX12Context> context = Renderer::GetContext<DirectX12Context>();
		m_DescriporSize = context->GetDevice()->GetDescriptorHandleIncrementSize(m_Type);

		uint32 size = m_Type == D3D12_DESCRIPTOR_HEAP_TYPE_SAMPLER ? D3D12_MAX_SHADER_VISIBLE_SAMPLER_HEAP_SIZE : D3D12_MAX_SHADER_VISIBLE_DESCRIPTOR_HEAP_SIZE_TIER_2;
		m_Capacity = size;
		m_FreeSlots.reserve(size);

		D3D12_DESCRIPTOR_HEAP_DESC desc;
		desc.Flags = m_IsShaderVisable ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
		desc.NumDescriptors = size;
		desc.Type = m_Type;
		desc.NodeMask = 0;
		CORE_ASSERT_HRESULT(context->GetDevice()->CreateDescriptorHeap(&desc, IID_PPV_ARGS(m_Heap.GetAddressOf())),
			"Failed to create descriptor heap");
		m_CPUHandle = m_Heap->GetCPUDescriptorHandleForHeapStart();
		if(isShaderVisable)
			m_GPUHandle = m_Heap->GetGPUDescriptorHandleForHeapStart();

		for (uint32 i = 0; i < size; i++) m_FreeSlots.push_back(i);

	}

	DirectX12DescriptorHeap::~DirectX12DescriptorHeap()
	{
		if(m_FreeSlots.size() < m_Capacity)
			CORE_WARN("Not all resorces have been freed prior to deleting descriptor heap");
	}

	DirectX12DescriptorHandle DirectX12DescriptorHeap::Allocate()
	{
		std::lock_guard g(m_Mutex);
		CORE_ASSERT(!m_FreeSlots.empty(), "Not Enough Memory");

		// get a free slot
		uint32 slot = m_FreeSlots.back();
		uint32 offset = slot * m_DescriporSize; // get the memory offset of the slot

		// create the handle
		DirectX12DescriptorHandle handle;
		handle.m_Container = this;
		handle.m_Index = slot;
		handle.cpu.ptr = m_CPUHandle.ptr + offset;
		if (m_IsShaderVisable)
			handle.gpu.ptr = m_GPUHandle.ptr + offset;

		// remove the slot from the list of free slots
		m_FreeSlots.pop_back();

		return handle;
	}

	void DirectX12DescriptorHeap::Free(DirectX12DescriptorHandle& handle)
	{
		if (!handle)
			return;

		m_FreeSlots.push_back(handle.m_Index); // add the slot to the list of free slots
		handle = {}; // invalidate the handle
	}

}
