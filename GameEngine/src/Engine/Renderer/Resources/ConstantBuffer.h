#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class ConstantBuffer
	{
	public:
		
		virtual void SetData(const void* data) = 0;
		virtual uint32 GetDescriptorLocation() = 0;

		static Ref<ConstantBuffer> Create(uint32 size);
	};
}