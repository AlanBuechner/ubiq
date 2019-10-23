#include "pch.h"
#include "Allocator.h"

namespace Engine
{
	Allocator::Allocator(size_t size)
		: m_Size(size), m_Used(0)
	{
	}
	Allocator::~Allocator()
	{
		m_Size = 0;
	}
}