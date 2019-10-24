/*
I got a lot of the code from https://github.com/mtrebi/memory-allocators
*/

#pragma once

#include "LinearAllocator.h"
#include "StackAllocator.h"

namespace Engine
{
	static LinearAllocator* LinearAlloc = new LinearAllocator(1000000000); // 1 Gigabyte of memory
	static StackAllocator* StackAlloc = new StackAllocator(1000000000); // 1 Gigabyte of memory

	template<class T>
	T* CreateObject()
	{
		void* M = LinearAlloc->allocate(sizeof(T), 4);
		M = new(M) T();
		return (T*)M;
	}
}