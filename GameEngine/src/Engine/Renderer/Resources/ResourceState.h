#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	enum ResourceState
	{
		Unknown,
		Common,
		ShaderResource,
		UnorderedResource,
		RenderTarget,
	};


	class GPUResource
	{
	public:
		virtual ~GPUResource() = default;

	public:
		ResourceState GetDefultState() { return m_DefultState; }

		virtual void* GetGPUResourcePointer() = 0;
	protected:
		virtual bool SupportState(ResourceState state) = 0;
		virtual uint32 GetState(ResourceState state) = 0;

	protected:
		ResourceState m_DefultState = ResourceState::Common;

		friend class DirectX12CommandList;
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
