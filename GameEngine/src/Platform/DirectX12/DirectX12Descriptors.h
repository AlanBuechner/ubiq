#pragma once
#include "DX.h"
#include "Engine/Core/Core.h"
#include <mutex>

namespace Engine
{
	class DirectX12DescriptorHeap;

	struct DirectX12DescriptorHandle
	{
	public:
		D3D12_CPU_DESCRIPTOR_HANDLE cpu;
		D3D12_GPU_DESCRIPTOR_HANDLE gpu;

		operator bool() const { return cpu.ptr != 0; }
		bool IsShaderVisible() const { return gpu.ptr != 0; }
		uint32 GetIndex() const { return m_Index; }

		void Free();

	private:
		friend DirectX12DescriptorHeap;
		DirectX12DescriptorHeap* m_Container = nullptr;
		uint32 m_Index = 0;
	};

	class DirectX12DescriptorHeap
	{
	public:
		DirectX12DescriptorHeap(D3D12_DESCRIPTOR_HEAP_TYPE type, bool isShaderVisible);
		~DirectX12DescriptorHeap();

		DirectX12DescriptorHandle Allocate();
		void Free(DirectX12DescriptorHandle& handle);

		D3D12_DESCRIPTOR_HEAP_TYPE GetType() const { return m_Type; }
		uint32 GetCapacity() { return m_Capacity; }
		uint32 GetSize() { return m_Capacity - (uint32)m_FreeSlots.size(); }
		wrl::ComPtr<ID3D12DescriptorHeap> GetHeap() { return m_Heap; }

		D3D12_CPU_DESCRIPTOR_HANDLE GetCPUHeapStart() { return m_CPUHandle; }
		D3D12_GPU_DESCRIPTOR_HANDLE GetGPUHeapStart() { return m_GPUHandle; }

	private:
		std::mutex m_Mutex;
		D3D12_DESCRIPTOR_HEAP_TYPE m_Type;
		wrl::ComPtr<ID3D12DescriptorHeap> m_Heap;
		D3D12_CPU_DESCRIPTOR_HANDLE m_CPUHandle;
		D3D12_GPU_DESCRIPTOR_HANDLE m_GPUHandle;
		std::vector<uint32> m_FreeSlots;
		uint32 m_Capacity = 0;
		uint32 m_DescriporSize;
		bool m_IsShaderVisable;
	};
}
