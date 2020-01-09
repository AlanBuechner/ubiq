#include "pch.h"
#include "FreeListAllocator.h"
#include "PointerMath.h"

namespace Engine
{
	FreeListAllocator::FreeListAllocator(size_t size, PlacementPolicy policy, uint8_t alignment)
		: Super(size), m_Policy(policy), m_Alignment(alignment)
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

		alignment = m_Alignment;

		size += allocationHeaderSize; // size of the allocation and the allocation header
		size += alignment - (size % alignment); // size is always aligned

		ASSERT(alignment >= 8, "aligment must be at least 8");

		size_t padding;
		Node* affectedNode;
		Node* previousNode;
		FindLocation(size, alignment, padding, previousNode, affectedNode); // finds a location in memory to put the new allocation
		ASSERT(affectedNode != nullptr, "not enough memory");

		const size_t alignmentPadding = padding - allocationHeaderSize;
		const size_t requiredSize = size + padding;

		const size_t rest = affectedNode->data.blockSize - requiredSize;

		if (rest > 0) {
			// We have to split the block into the data block and a free block of size 'rest'
			Node* newFreeNode = (Node*)((size_t)affectedNode + requiredSize);
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

		allocations.push_back(dataAddress);

		return (void*)dataAddress;
	}

	void FreeListAllocator::Deallocate(void* p)
	{
		// Insert it in a sorted position by the address number
		const size_t currentAddress = (size_t) p;
		const size_t headerAddress = currentAddress - sizeof(FreeListAllocator::AllocationHeader); // the address of the allocation header
		const FreeListAllocator::AllocationHeader* allocationHeader{ (FreeListAllocator::AllocationHeader*) headerAddress }; // crates copy of the allocation header

		Node* freeNode = (Node*)(headerAddress); // the node that needs to be freed
		freeNode->data.blockSize = allocationHeader->blockSize + allocationHeader->padding; // sets the size of the node that is being freed
		freeNode->next = nullptr;

		// find the previus node in the linked list and free the node we are dealocating
		Node* it = m_FreeList.head; // current node is head of the linked list
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

		// remove allocation form the allocations array
		for (int i = 0; i < allocations.size(); i++)
		{
			if ((void*)allocations[i] == p)
			{
				allocations.erase(allocations.begin() + i);
				break;
			}
		}

		// Merge contiguous nodes
		Coalescence(itPrev, freeNode);
	}

	bool FreeListAllocator::ResizeAllocation(void* p, size_t newSize)
	{
		AllocationHeader* header = (AllocationHeader*)((size_t)p - sizeof(AllocationHeader)); // allocation header
		Node* freeListNode = (Node*)((size_t)header + header->blockSize); // if a free list node is at the end it will be hear

		size_t size = header->blockSize - sizeof(Node) - sizeof(AllocationHeader);

		if (size == newSize)
			return true; // the allocaton is allredy the requierd size

		if (size < newSize) // if we are incresing the size of the allocaton
		{

			Node* it = m_FreeList.head; // the start of the free list
			Node* itPrev = nullptr;
			while (it != nullptr)
			{
				if (it == freeListNode) // if the node is the one at the end of the allocaton
				{
					size_t bytesAdded = newSize - size;
					size_t requierdSizeToAdd = bytesAdded + (bytesAdded%m_Alignment);
					size_t requierdSize = size + requierdSizeToAdd + sizeof(AllocationHeader) + sizeof(Node);
					if (it->data.blockSize > requierdSizeToAdd + sizeof(Node))
					{
						Node* next = it->next;
						header->blockSize = requierdSize; // resize the allocation
						size_t rest = it->data.blockSize - requierdSize; // find the rest of the free node
						Node* newFreeNode = (Node*)((size_t)header + requierdSize); // create the free node
						newFreeNode->data.blockSize = rest; // set the free nodes size
						newFreeNode->next = next; // set the next node
						if(itPrev != nullptr) // has previous node
							itPrev->next = newFreeNode; // set the previous nodes next to the new node;
						else // head of the list
							m_FreeList.head = newFreeNode; // set the head to the new node
						return true;
					}
					else
						return false; // not enough space to resize
				}

				itPrev = it;
				it = it->next;
			}
			return false; // no node after the allocation so no resizeing can be done
		}
		else // if we are decresing the allocation size
		{
			return false; // not implemented
		}
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

	void FreeListAllocator::StartMemoryDebuging(const UString& name, const UString& path)
	{
		m_Name = name.RawString();
		m_OutputStream.open(path.RawString());
		m_OutputStream << "{";
		m_OutputStream << "\"type\" : \"FreeList\",";
		m_OutputStream << "\"allocator\" :[{";
		m_OutputStream << "\"id\" : 1,";
		m_OutputStream << "\"name\" : \"" << name << "\",";
		m_OutputStream << "\"size\" : " << m_Size << ",";
		m_OutputStream << "\"headerSize\" : " << (uint32_t)sizeof(AllocationHeader) << ",";
		m_OutputStream << "\"listHeaderSize\" : " << (uint32_t)sizeof(FreeHeader) << ",";
		m_OutputStream << "\"snapShots\" :[";
		m_OutputStream.flush();
	}

	uint16_t snapShot = 0;

	void FreeListAllocator::TakeSnapShot()
	{
		if (!m_OutputStream.is_open())
			return;

		if (snapShot != 0)
			m_OutputStream << ",";
		snapShot++;

		m_OutputStream << "{";
		m_OutputStream << "\"alloc\":[";

		for (int i = 0; i < allocations.size(); i++)
		{
			FreeListAllocator::AllocationHeader* header = (FreeListAllocator::AllocationHeader*)(allocations[i] - sizeof(AllocationHeader));

			m_OutputStream << "{";
			m_OutputStream << "\"start\":" << (uint32_t)((size_t)header - (size_t)m_Start) << ","; // the start of the allocation
			m_OutputStream << "\"header\":[{"; // the header memory
			uint32_t size = (uint32_t)(header->blockSize - sizeof(Node));
			m_OutputStream << "\"size\":" << size << ","; // the size of the allocation
			m_OutputStream << "\"padding\":" << (uint32_t)header->padding << " }],"; // the padding
			m_OutputStream << "\"body\":\"" << "" << "\""; // the information in the allocation
			m_OutputStream << "}"; // close the allocation
			m_OutputStream.flush();

			if (i < allocations.size()-1)
				m_OutputStream << ",";
		}

		// close allocation
		m_OutputStream << "],\"linkedList\":[";

		Node* it = m_FreeList.head; // current node is head of the linked list
		Node* itPrev = nullptr;
		while (it != nullptr)
		{
			m_OutputStream << "{";
			m_OutputStream << "\"start\":" << (uint32_t)((size_t)it- (size_t)m_Start) << ",";
			m_OutputStream << "\"next\":" << (uint32_t)((it->next == nullptr ? 0 : ((size_t)it->next - (size_t)m_Start)));
			m_OutputStream << "}";
			m_OutputStream.flush();

			itPrev = it;
			it = it->next;
			if (it != nullptr)
			{
				size_t dist = (size_t)it - (size_t)m_Start;
				if (dist > m_Size)
				{
					DEBUG_ERROR("{0} is out of bounds", (size_t)it);
					break;
				}
				m_OutputStream << ",";
			}
		}

		// close snapShot
		m_OutputStream << "]}";
		m_OutputStream.flush();
	}

	void FreeListAllocator::StopMemoryDebuging()
	{
		m_OutputStream << "]}]}";

		m_OutputStream.close();
	}

	void FreeListAllocator::Coalescence(Node* previousNode, Node* freeNode)
	{
		if (freeNode->next != nullptr &&
			(std::size_t) freeNode + freeNode->data.blockSize == (std::size_t) freeNode->next)
		{
			freeNode->data.blockSize += freeNode->next->data.blockSize;
			m_FreeList.remove(freeNode, freeNode->next);
		}
		if (previousNode != nullptr) 
		{
			size_t dist = (std::size_t)previousNode + previousNode->data.blockSize - (size_t)m_Start;
			if (dist + (size_t)m_Start == (std::size_t) freeNode)
			{
				previousNode->data.blockSize += freeNode->data.blockSize;
				m_FreeList.remove(previousNode, freeNode);
			}
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
			FindBestLocation(size, alignment, padding, previousNode, foundNode); // Need to fix
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
			padding = PointerMath::CalculatePaddingWithHeader((std::size_t)it, alignment, sizeof(FreeListAllocator::AllocationHeader));
			const std::size_t requiredSpace = size + padding + sizeof(Node);
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
