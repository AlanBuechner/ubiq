#pragma once
#include "Allocator.h"
#include "SinglyLinkedList.h"
#include "Engine/Util/UString.h"
#include <iostream>
#include <fstream>
#include <vector>

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

		std::vector<size_t> allocations;

		// memory debuging
		std::ofstream m_OutputStream;
		char* m_Name;

		const uint8_t m_Alignment;
		uint16_t snapShot = 0;
	public:
		FreeListAllocator(size_t size, PlacementPolicy policy, uint8_t alignment);
		virtual ~FreeListAllocator();

		virtual void Init() override;
		virtual void* Allocate(size_t size, uint8_t alignment = 0) override; // dont use alignment
		virtual void Deallocate(void* p) override;
		bool ResizeAllocation(void* p, size_t newSize);
		virtual void Reset();

		virtual void StartMemoryDebuging(const UString& name, const UString& path);
		virtual void TakeSnapShot();
		virtual void StopMemoryDebuging();
	private:
		FreeListAllocator(FreeListAllocator& freeListAllocator);

		void Coalescence(Node* previousNode, Node* freeNode);

		void FindLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
		void FindBestLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
		void FindFirstLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode);
	};
}