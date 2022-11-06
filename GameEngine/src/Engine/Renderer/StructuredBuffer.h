#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class StructuredBuffer
	{
	public:
		virtual void Resize(uint32 count) = 0;
		virtual void SetData(void* data, uint32 count = 1, uint32 start = 0) = 0;

		static Ref<StructuredBuffer> Create(uint32 stride, uint32 count);
	};
}
