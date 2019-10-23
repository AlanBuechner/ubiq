#pragma once
#include <cstdint>
#include "Engine/Core/core.h"

namespace Engine
{
	class Allocator
	{
	public:
		Allocator(size_t size);
		virtual ~Allocator();

		virtual void* allocate(size_t size, uint8_t alignment = 4) = 0;
		virtual void deallocate() = 0;
		virtual void Init() = 0;
		size_t getSize() const { return m_Size; }

	protected:
		size_t m_Size;
		size_t m_Used;
		size_t m_Peak;

		typedef Allocator Super;
	};
}