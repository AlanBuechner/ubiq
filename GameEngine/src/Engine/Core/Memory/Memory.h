/*
I got a lot of the code from https://github.com/mtrebi/memory-allocators
*/

#pragma once

#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"

namespace Engine
{
	static LinearAllocator* LinearAlloc = new LinearAllocator(1000000000); // 1 Gigabyte of memory
	static StackAllocator* StackAlloc = new StackAllocator(1000000000); // 1 Gigabyte of memory
	//static PoolAllocator* PoolAlloc = new PoolAllocator(1000000000, sizeof(std::string) + sizeof(int) + sizeof(float)); // 1 Gigabyte of memory
	static FreeListAllocator* FreeAlloc = new FreeListAllocator(1000000000, Engine::FreeListAllocator::PlacementPolicy::FindFirst);


	template<typename T>
	class SharedPtr
	{
		struct Obj
		{
			uint8_t m_Count = 0;
			T* m_Object = nullptr;
			Allocator* m_Alloc = nullptr;
		}* m_Object;

	public:
		SharedPtr(Obj* object = nullptr)
		{
			m_Object = object;
			if(m_Object != nullptr)
				m_Object->m_Count++;
		}

		SharedPtr(const SharedPtr<T>& ptr)
		{
			m_Object = ptr.m_Object;
			if(m_Object != nullptr)
				m_Object->m_Count++;
		}

		~SharedPtr()
		{
			if (m_Object != nullptr)
			{
				m_Object->m_Count--;
				if (m_Object->m_Count == 0)
				{
					m_Object->m_Alloc->Deallocate(m_Object->m_Object);
					m_Object->m_Alloc->Deallocate(m_Object);
				}
			}
		}

		void operator=(const SharedPtr<T>& ptr)
		{
			if (m_Object != nullptr)
			{
				m_Object->m_Count--;
			}

			m_Object = ptr.m_Object;
			if (m_Object != nullptr)
			{
				m_Object->m_Count++;
			}
		}

		static Obj* Create(Allocator& alloc, uint8_t alignment = 8)
		{
			Obj* start = (Obj*)alloc.Allocate(sizeof(Obj), alignment);
			start = new(start) Obj();
			start->m_Alloc = &alloc;
			start->m_Object = (T*)alloc.Allocate(sizeof(T), alignment);
			start->m_Object = new(start->m_Object) T();
			return start;
		}

		T* operator->()
		{
			return m_Object->m_Object;
		}
	};

	template<class T>
	T* CreateObject()
	{
		T* M = (T*)FreeAlloc->Allocate(sizeof(T), 8);
		M = new(M) T();
		return M;
	}
}