#include "pch.h"
#include "FreeListAllocator.h"
#include "PointerMath.h"

namespace Engine
{
	FreeListAllocator::FreeListAllocator(size_t size, PlacementPolicy policy)
		: Super(size), m_Policy(policy)
	{
		Init();
	}

	FreeListAllocator::~FreeListAllocator()
	{
		Deallocate(m_Start);
		m_Start = nullptr;
	}

	void FreeListAllocator::Init()
	{
		if (m_Start != nullptr)
		{
			Deallocate(m_Start);
			m_Start = nullptr;
		}
		m_Start = malloc(m_Size);
		Reset();
	}

	void* FreeListAllocator::Allocate(size_t size, uint8_t alignment)
	{
		const size_t allocationHeaderSize = sizeof(FreeListAllocator::AllocationHeader);
		const size_t freeHeaderSize = sizeof(FreeListAllocator::FreeHeader);

		size += sizeof(Node);

		ASSERT(size >= sizeof(Node) ,"Allocation size must be bigger");
		ASSERT(alignment >= 8, "aligment must be at least 8");

		size_t padding;
		Node* affectedNode;
		Node* previousNode;
		FindLocation(size, alignment, padding, previousNode, affectedNode);
		ASSERT(affectedNode != nullptr, "not enough memory");

		const size_t alignmentPadding = padding - allocationHeaderSize;
		const size_t requiredSize = size + padding;

		const size_t rest = affectedNode->data.blockSize - requiredSize;

		if (rest > 0) {
			// We have to split the block into the data block and a free block of size 'rest'
			Node* newFreeNode = (Node*)((size_t) affectedNode + requiredSize);
			newFreeNode->data.blockSize = rest;
			m_FreeList.insert(affectedNode, newFreeNode);
		}
		m_FreeList.remove(previousNode, affectedNode);

		const size_t headerAddress = (size_t) affectedNode + alignmentPadding;
		const size_t dataAddress = headerAddress + allocationHeaderSize;
		((FreeListAllocator::AllocationHeader*) headerAddress)->blockSize = requiredSize;
		((FreeListAllocator::AllocationHeader*) headerAddress)->padding = (char)alignmentPadding;

		m_Used += requiredSize;
		m_Peak = std::max(m_Peak, m_Used);

		return (void*)dataAddress;
	}

	void FreeListAllocator::Deallocate(void* p)
	{
		// Insert it in a sorted position by the address number
		const size_t currentAddress = (size_t) p;
		const size_t headerAddress = currentAddress - sizeof(FreeListAllocator::AllocationHeader);
		const FreeListAllocator::AllocationHeader* allocationHeader{ (FreeListAllocator::AllocationHeader*) headerAddress };

		Node* freeNode = (Node*)(headerAddress);
		freeNode->data.blockSize = allocationHeader->blockSize + allocationHeader->padding;
		freeNode->next = nullptr;

		Node* it = m_FreeList.head;
		Node* itPrev = nullptr;
		while (it != nullptr) {
			if (p < it) {
				m_FreeList.insert(itPrev, freeNode);
				break;
			}
			itPrev = it;
			it = it->next;
		}

		m_Used -= freeNode->data.blockSize;

		// Merge contiguous nodes
		Coalescence(itPrev, freeNode);
	}

	void FreeListAllocator::Reset()
	{
		m_Used = 0;
		m_Peak = 0;
		Node* firstNode = (Node*)m_Start;
		firstNode->data.blockSize = m_Size;
		firstNode->next = nullptr;
		m_FreeList.head = nullptr;
		m_FreeList.insert(nullptr, firstNode);
	}

	void FreeListAllocator::Coalescence(Node* previousNode, Node* freeNode)
	{
		if (freeNode->next != nullptr &&
			(std::size_t) freeNode + freeNode->data.blockSize == (std::size_t) freeNode->next) {
			freeNode->data.blockSize += freeNode->next->data.blockSize;
			m_FreeList.remove(freeNode, freeNode->next);
		}

		if (previousNode != nullptr &&
			(std::size_t) previousNode + previousNode->data.blockSize == (std::size_t) freeNode) {
			previousNode->data.blockSize += freeNode->data.blockSize;
			m_FreeList.remove(previousNode, freeNode);
		}
	}

	void FreeListAllocator::FindLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode)
	{
		switch (m_Policy)
		{
		case FreeListAllocator::PlacementPolicy::FindFirst:
			FindFirstLocation(size, alignment, padding, previousNode, foundNode);
			break;
		case FreeListAllocator::PlacementPolicy::FindBest:
			//FindBestLocation(size, alignment, padding, previousNode, foundNode); // Need to fix
			break;
		default:
			break;
		}
	}

	void FreeListAllocator::FindBestLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode)
	{
		// Iterate WHOLE list keeping a pointer to the best fit
		size_t smallestDiff = std::numeric_limits<size_t>::max();
		Node* bestBlock = nullptr;
		Node* it = m_FreeList.head,
			* itPrev = nullptr;
		while (it != nullptr) {
			padding = PointerMath::CalculatePaddingWithHeader((size_t)it, alignment, sizeof(FreeListAllocator::AllocationHeader));
			const size_t requiredSpace = size + padding;
			if (it->data.blockSize >= requiredSpace && (it->data.blockSize - requiredSpace < smallestDiff)) {
				bestBlock = it;
			}
			itPrev = it;
			it = it->next;
		}
		previousNode = itPrev;
		foundNode = bestBlock;
	}

	void FreeListAllocator::FindFirstLocation(const size_t size, const size_t alignment, size_t& padding, Node*& previousNode, Node*& foundNode)
	{
		//Iterate list and return the first free block with a size >= than given size
		Node* it = m_FreeList.head,
			* itPrev = nullptr;

		while (it != nullptr) {
			padding = PointerMath::CalculatePaddingWithHeader((size_t)it, alignment, sizeof(FreeListAllocator::AllocationHeader));
			const size_t requiredSpace = size + padding;
			if (it->data.blockSize >= requiredSpace) {
				break;
			}
			itPrev = it;
			it = it->next;
		}
		previousNode = itPrev;
		foundNode = it;
	}

}
