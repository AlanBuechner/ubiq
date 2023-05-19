#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	enum ResourceState
	{
		Common,
		ShaderResource,
		UnorderedResource,
		RenderTarget
	};

	class GPUResource
	{
	protected:
		virtual void* GetGPUResourcePointer() = 0;
		virtual bool SupportState(ResourceState state) = 0;
		virtual uint32 GetState(ResourceState state) = 0;

		friend class DirectX12CommandList;
	};

	struct ResourceTransitionObject
	{
		Ref<GPUResource> resource;
		ResourceState to, from;
	};
}
