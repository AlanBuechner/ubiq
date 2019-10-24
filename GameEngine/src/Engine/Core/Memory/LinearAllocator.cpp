#include "pch.h"
#include "LinearAllocator.h"
#include "PointerMath.h"
#include <algorithm>

namespace Engine
{
	LinearAllocator::LinearAllocator(size_t size)
		: Super(size)
	{
		Init();
	}

	LinearAllocator::~LinearAllocator()
	{
		free(m_Start);
		m_Start = nullptr;
	}

	void LinearAllocator::Init()
	{
		if (m_Start != nullptr) 
		{
			Reset();
		}
		m_Start = malloc(m_Size);
		m_Offset = 0;
	}

	void* LinearAllocator::Allocate(size_t size, uint8_t alignment)
	{
		size_t padding = 0;
		size_t paddedAddress = 0;
		const size_t currentAddress = (size_t)m_Start + m_Offset;

		if (alignment != 0 && m_Offset % alignment != 0) 
		{
			// Alignment is required. Find the next aligned memory address and update offset
			padding = PointerMath::CalculatePadding(currentAddress, alignment);
		}

		if (m_Offset + padding + size > m_Size) 
		{
			return nullptr;
		}

		m_Offset += padding;
		const size_t nextAddress = currentAddress + padding;
		m_Offset += size;

		m_Used = m_Offset;
		m_Peak = std::max(m_Peak, m_Used);

		return (void*)nextAddress;
	}

	void LinearAllocator::Deallocate(void* p)
	{
		
	}

	void LinearAllocator::Reset()
	{
		m_Offset = 0;
		m_Used = 0;
		m_Peak = 0;
	}

}