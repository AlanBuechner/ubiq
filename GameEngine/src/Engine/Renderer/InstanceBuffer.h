#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class InstanceBuffer
	{
	public:

		virtual void SetData(uint32 start, uint32 count, const void* data) = 0;
		virtual uint32 GetCount() = 0;

		static Ref<InstanceBuffer> Create(uint32 stride, uint32 count);
	};
}
