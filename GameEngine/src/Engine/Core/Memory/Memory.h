#pragma once

#include "LinearAllocator.h"
#include <initializer_list>

namespace Engine
{
	static LinearAllocator* Alloc = new LinearAllocator(SIZE_MAX);

	template<class T>
	T* CreateObject()
	{
		T* M = (T*)Alloc->allocate(sizeof(T), 4);
		M = new T();
		return M;
	}
}