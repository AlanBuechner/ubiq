#pragma once
#include "Allocator.h"
#include "SinglyLinkedList.h"

namespace Engine
{
	class FreeListAllocator : public Allocator
	{
	public:
		enum PlacementPolicy {
			FindFirst,
			FindBest // do not use
		};

	private:
		struct FreeHeader {
			size_t blockSize;
		};
		struct AllocationHeader {
			size_t blockSize;
			char padding;
		};

		typedef SinglyLinkedList<FreeHeader>::Node Node;


		void* m_Start = nullptr;
		PlacementPolicy m_Policy;
		SinglyLinkedList<FreeHeader> m_FreeList;
	public:
		FreeListAllocator(size_t size, PlacementPolicy policy );
		virtual ~FreeListAllocator();

		virtual void Init() override;
		virtual void* Allocate(size_t size, uint8_t alignment) override;
		virtual void Deallocate(void* p) override;
		virtual void Reset();
	private:
		FreeListAllocator(FreeListAllocator& freeListAllocator);

		void Coalescence(Node* previousNode, Node* freeNode);

		void FindLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
		void FindBestLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
		void FindFirstLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
	};
}