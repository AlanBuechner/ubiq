#pragma once
#include "Allocator.h"

namespace Engine
{
	class LinearAllocator : public Allocator
	{
	public:
		LinearAllocator(size_t size);
		virtual ~LinearAllocator();

		virtual void Init() override;
		virtual void* Allocate(size_t size, uint8_t alignment) override;
		virtual void Deallocate(void* p) override;
		virtual void Reset();
	protected:
		void* m_Start = nullptr;
		size_t m_Offset;
	private:
		LinearAllocator(LinearAllocator& linearAllocator);
	};
}