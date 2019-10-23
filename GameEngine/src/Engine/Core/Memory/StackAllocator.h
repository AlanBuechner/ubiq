#pragma once
#include "Allocator.h"

namespace Engine
{
	class StackAllocator : public Allocator
	{
	public:
		StackAllocator(size_t size);
		virtual ~StackAllocator();

		virtual void Init() override;
		virtual void* allocate(size_t size, uint8_t alignment) override;
		virtual void deallocate(void* p) override;
		virtual void Reset();
	protected:
		void* m_Start = nullptr;
		size_t m_Offset;
	private:
		StackAllocator(StackAllocator& linearAllocator);

		struct AllocationHeader {
			char padding;
		};
	};
}