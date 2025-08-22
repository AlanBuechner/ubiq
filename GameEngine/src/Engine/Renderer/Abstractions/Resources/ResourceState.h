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
		virtual ~GPUResource() = default;

	public:
		ResourceState GetDefultState() { return m_DefultState; }

		virtual void* GetGPUResourcePointer() = 0;
		virtual bool SupportState(ResourceState state) = 0;
		virtual uint32 GetGPUState(ResourceState state) = 0;
		uint32 GetUnderlyingResourceSize() { return m_UnderlyingResourceSize; }
		bool IsTransient() { return m_Transient; }
		virtual void AllocateTransient(class TransientResourceHeap* heap, uint32 offset) = 0;

	protected:
		uint32 m_UnderlyingResourceSize = 0;
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
