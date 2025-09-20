#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	enum ResourceState
	{
		Unknown,
		Common,
		PiplineInput,
		ShaderResource,
		UnorderedResource,
		RenderTarget,
		CopySource,
		CopyDestination
	};


	class GPUResource
	{
	public:
		struct AllocationInfo
		{
			uint32 size = 0;
			uint32 allignment = 0;
		};

	public:
		virtual ~GPUResource() = default;

	public:
		ResourceState GetDefultState() const { return m_DefultState; }

		virtual void* GetGPUResourcePointer() const = 0;
		virtual bool SupportState(ResourceState state) const  = 0;
		virtual uint32 GetGPUState(ResourceState state) const = 0;
		AllocationInfo GetAllocationInfo() const { return m_AllocationInfo; }
		bool IsTransient() const { return m_Transient; }
		virtual void AllocateTransient(class TransientResourceHeap* heap, uint32 offset) = 0;

	protected:
		AllocationInfo m_AllocationInfo;
		bool m_Transient = false;
		ResourceState m_DefultState = ResourceState::Common;
		ResourceState m_EOFState = ResourceState::Unknown; // end of frame state

		friend class CPUCommandAllocator;
	};

	using GPUResourceHandle = GPUResource* const*;

	struct ResourceTransitionObject
	{
		GPUResource* resource;
		ResourceState to, from;
	};

	struct ResourceStateObject
	{
		GPUResource* resource;
		ResourceState state;
	};
}
