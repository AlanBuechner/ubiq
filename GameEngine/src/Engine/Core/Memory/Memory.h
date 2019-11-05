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
	static FreeListAllocator* DefalteAlloc = new FreeListAllocator(1000000000, Engine::FreeListAllocator::PlacementPolicy::FindFirst);

	static struct RefCountedObj
	{
		uint8_t m_Count = 0;
		void* m_Object = nullptr;
		Allocator* m_Alloc = nullptr;
	};

	template<class T>
	class SharedPtr
	{
		RefCountedObj* m_Object = nullptr;

	public:
		SharedPtr(RefCountedObj* object = nullptr)
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

		void Reset(T* obj)
		{
			if (m_Object == nullptr)
			{
				*this = CreateSharedPtrE<T>();
			}
			m_Object->m_Object = obj;
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

		T* operator->() const
		{
			return (T*)m_Object->m_Object;
		}
		
		T& operator*() const
		{
			return *(T*)m_Object->m_Object;
		}
		
		template<class B>
		operator SharedPtr<B>()
		{
			if (std::is_base_of<B, T>::value)
				return SharedPtr<B>(m_Object);
			return nullptr;
		}
	};

	template<class T, class... _Ty>
	SharedPtr<T> CreateSharedPtr(_Ty&& ... params)
	{
		size_t size = sizeof(RefCountedObj);
		RefCountedObj* start = (RefCountedObj*)DefalteAlloc->Allocate(size, 8);
		start = new(start) RefCountedObj();
		start->m_Alloc = DefalteAlloc;
		start->m_Object = (T*)DefalteAlloc->Allocate(sizeof(T), 8);
		start->m_Object = new(start->m_Object) T(std::forward<_Ty>(params)...);
		return SharedPtr<T>(start);
	}

	template<class T>
	SharedPtr<T> CreateSharedPtrE()
	{
		size_t size = sizeof(RefCountedObj);
		RefCountedObj* start = (RefCountedObj*)DefalteAlloc->Allocate(size, 8);
		start = new(start) RefCountedObj();
		start->m_Alloc = DefalteAlloc;
		start->m_Object = (T*)DefalteAlloc->Allocate(sizeof(T), 8);
		start->m_Object = nullptr;
		return SharedPtr<T>(start);
	}

	template<class T>
	T* CreateObject()
	{
		T* M = (T*)FreeAlloc->Allocate(sizeof(T), 8);
		M = new(M) T();
		return M;
	}
}