#include "pch.h"
#include "PoolAllocator.h"

namespace Engine
{
	PoolAllocator::PoolAllocator(size_t size, size_t chuckSize)
		: Super(size), m_ChunkSize(chuckSize)
	{
		Init();
	}

	void PoolAllocator::Init()
	{
		m_Start = malloc(m_Size);
		Reset();
	}

	void* PoolAllocator::Allocate(size_t size, uint8_t allinment)
	{
		ASSERT(size == m_ChunkSize, "allocation size must be the same as the chuck size");

		Node* freePosition = m_FreeList.pop();

		m_Used += m_ChunkSize;
		m_Peak = std::max(m_Peak, m_Used);

		return (void*)freePosition;
	}

	void PoolAllocator::Deallocate(void* p)
	{
		m_Used -= m_ChunkSize;
		m_FreeList.push((Node*)p);
	}

	void PoolAllocator::Reset()
	{
		m_Used = 0;
		m_Peak = 0;
		// Create a linked-list with all free positions
		const int nChunks = m_Size / m_ChunkSize;
		for (int i = 0; i < nChunks; ++i) {
			size_t address = (size_t) m_Start + i * m_ChunkSize;
			m_FreeList.push((Node*)address);
		}
	}

}