/*
I got a lot of the code from https://github.com/mtrebi/memory-allocators
*/

#pragma once

#include "LinearAllocator.h"
#include "StackAllocator.h"

namespace Engine
{
	static LinearAllocator* LinearAlloc = new LinearAllocator(SIZE_MAX);
	static StackAllocator* StackAlloc = new StackAllocator(SIZE_MAX);

	template<class T>
	T* CreateObject()
	{
		T* M = (T*)StackAlloc->allocate(sizeof(T), 4);
		M = new T();
		return M;
	}
}