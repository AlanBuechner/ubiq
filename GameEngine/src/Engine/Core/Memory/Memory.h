/*
I got a lot of the code from https://github.com/mtrebi/memory-allocators
*/

#pragma once

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"

namespace Engine
{
	static LinearAllocator* LinearAlloc = new LinearAllocator(1000000000); // 1 Gigabyte of memory
	static StackAllocator* StackAlloc = new StackAllocator(1000000000); // 1 Gigabyte of memory
	static PoolAllocator* PoolAlloc = new PoolAllocator(1000000000, sizeof(std::string) + sizeof(int) + sizeof(float)); // 1 Gigabyte of memory

	template<class T>
	T* CreateObject()
	{
		T* M = (T*)PoolAlloc->Allocate(sizeof(T), 4);
		*M = T();
		return M;
	}
}