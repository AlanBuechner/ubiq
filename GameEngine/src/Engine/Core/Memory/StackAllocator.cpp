#include "pch.h"
#include "StackAllocator.h"
#include "PointerMath.h"

namespace Engine
{

	StackAllocator::StackAllocator(size_t size)
		: Super(size)
	{
		Init();
	}

	StackAllocator::~StackAllocator()
	{
		Reset();
		m_Start = nullptr;
	}

	void StackAllocator::Init()
	{
		if (m_Start != nullptr)
		{
			Reset();
		}
		m_Start = malloc(m_Size); // alocats memory for the memory allocator to allocat for other objects 
		m_Offset = 0; // inital offset of 0
	}

	void* StackAllocator::Allocate(size_t size, uint8_t alignment)
	{
		const size_t currentAddress = (std::size_t)m_Start + m_Offset; // gets tge current address

		size_t padding = PointerMath::CalculatePaddingWithHeader(currentAddress, alignment, sizeof(AllocationHeader)); // gets the padding for the memory regen to be allined

		// checks if the allocator isent allocating more memory than it has
		if (m_Offset + padding + size > m_Size) 
		{
			return nullptr;
		}
		m_Offset += padding; // sets the offset to be allined

		const size_t nextAddress = currentAddress + padding; // gets the address of the new block of memory
		const size_t headerAddress = nextAddress - sizeof(AllocationHeader); // gets the addres of the header
		AllocationHeader allocationHeader{ (char)padding };
		AllocationHeader* headerPtr = (AllocationHeader*)headerAddress;
		headerPtr = &allocationHeader;

		m_Offset += size; // sets the new offset for the next block of memroy to be allocated

		m_Used = m_Offset; // setst he amount of memory being used
		m_Peak = std::max(m_Peak, m_Used);

		return (void*)nextAddress;
	}

	void StackAllocator::Deallocate(void* p)
	{
		// Move offset back to clear address
		const std::size_t currentAddress = (std::size_t) p;
		const std::size_t headerAddress = currentAddress - sizeof(AllocationHeader); // gets the header addres
		const AllocationHeader* allocationHeader{ (AllocationHeader*)headerAddress }; // gets the header

		m_Offset = currentAddress - allocationHeader->padding - (std::size_t) m_Start; // removes the block of memory form the stack
		m_Used = m_Offset; // sets the used memory
	}

	void StackAllocator::Reset()
	{
		m_Offset = 0;
		m_Peak = 0;
		m_Used = 0;
	}

}