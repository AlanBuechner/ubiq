#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class ConstantBuffer
	{
	public:
		
		virtual void SetData(void* data) = 0;

		static Ref<ConstantBuffer> Create(uint32 size);
	};
}
