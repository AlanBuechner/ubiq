#pragma once
#include "Allocator.h"
#include "StackLinkedList.h"

namespace Engine
{
	class PoolAllocator : public Allocator
	{
	public:
		PoolAllocator(size_t size, size_t chuckSize);

		virtual void Init() override;

		virtual void* Allocate(size_t size, uint8_t allinment) override;
		virtual void Deallocate(void* p) override;
		virtual void Reset();

	protected:
		void* m_Start = nullptr;
		size_t m_ChunkSize;

	private:
		struct FreeHeader
		{

		};
		typedef Engine::StackLinkedList<FreeHeader>::Node Node;
		StackLinkedList<FreeHeader> m_FreeList;
	};
}