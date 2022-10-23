#pragma once
#include "Engine/Core/Core.h"

namespace Engine
{
	class InstanceBuffer
	{
	public:

		virtual void SetData(uint32 start, uint32 count, const void* data) = 0;
		virtual void PushBack(uint32 count, const void* data) = 0;
		virtual void PopBack() = 0;
		virtual void Clear() = 0;
		virtual void Apply() = 0;
		template<typename T>
		const T& Get(uint32 index) { return *(T*)At(index); } // can get but cant wright to the memory location use set data to write to memory
		virtual void* At(uint32 index) = 0;
		virtual uint32 GetCount() = 0;
		bool Empty() { return GetCount() == 0; }

		static Ref<InstanceBuffer> Create(uint32 stride, uint32 count);
	};
}
