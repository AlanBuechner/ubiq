#pragma once

/*
I got a lot of the allocator code from https://github.com/mtrebi/memory-allocators
*/
#include "LinearAllocator.h"
#include "StackAllocator.h"
#include "PoolAllocator.h"
#include "FreeListAllocator.h"

namespace Engine
{
	static FreeListAllocator* DefalteAlloc = new FreeListAllocator(1000000000, Engine::FreeListAllocator::PlacementPolicy::FindFirst);

	struct RefCountedObj
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
				if (m_Object->m_Object != nullptr)
				{
					m_Object->m_Count--;
					if (m_Object->m_Count == 0)
					{
						m_Object->m_Alloc->Deallocate(m_Object->m_Object);
						m_Object->m_Alloc->Deallocate(m_Object);
					}
				}
			}
		}

		void Reset(T* obj)
		{
			if (m_Object == nullptr)
			{
				*this = CreateSharedPtrE<T>();
			}
			if (m_Object->m_Object != nullptr)
			{
				m_Object->m_Alloc->Deallocate(m_Object->m_Object);
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
	SharedPtr<T> CreateSharedPtr(T* obj)
	{
		size_t size = sizeof(RefCountedObj);
		RefCountedObj* start = (RefCountedObj*)DefalteAlloc->Allocate(size, 8);
		start = new(start) RefCountedObj();
		start->m_Alloc = DefalteAlloc;
		start->m_Object = obj;
		return SharedPtr<T>(start);
	}

	template<class T>
	SharedPtr<T> CreateSharedPtrE()
	{
		return CreateSharedPtr<T>(nullptr);
	}

	struct ScopedObj
	{
		void* m_Object = nullptr;
		Allocator* m_Alloc = nullptr;
	};

	template<class T>
	class ScopedPtr
	{
		ScopedObj* m_Object;

	public:
		ScopedPtr(ScopedObj* obj = nullptr)
		{
			m_Object = obj;
		}

		ScopedPtr(const ScopedPtr& ptr);

		~ScopedPtr()
		{
			if (m_Object != nullptr)
			{
				if (m_Object->m_Object != nullptr)
				{
					m_Object->m_Alloc->Deallocate(m_Object->m_Object);
					m_Object->m_Alloc->Deallocate(m_Object);
					m_Object = nullptr;
				}
			}
		}

		void MoveOwnership(ScopedPtr& ptr)
		{
			ptr.m_Object = m_Object;
			m_Object = nullptr;
		}

		T& operator*()
		{
			return *(T*)m_Object->m_Object;
		}

		T* operator->()
		{
			return (T*)m_Object->m_Object;
		}
	};

	template<class T, class... _Ty>
	ScopedPtr<T> CreateScopedPtr(_Ty&& ... params)
	{
		size_t size = sizeof(ScopedObj);
		ScopedObj* start = (ScopedObj*)DefalteAlloc->Allocate(size, 8);
		start = new(start) ScopedObj();
		start->m_Alloc = DefalteAlloc;
		start->m_Object = (T*)DefalteAlloc->Allocate(sizeof(T), 8);
		start->m_Object = new(start->m_Object) T(std::forward<_Ty>(params)...);
		return ScopedPtr<T>(start);
	}

	template<class T>
	ScopedPtr<T> CreateScopedPtr(T* obj)
	{
		size_t size = sizeof(ScopedObj);
		ScopedObj* start = (ScopedObj*)DefalteAlloc->Allocate(size, 8);
		start = new(start) ScopedObj();
		start->m_Alloc = DefalteAlloc;
		start->m_Object = obj;
		return ScopedPtr<T>(start);
	}

	template<class T, class... _Ty>
	ScopedPtr<T> CreateScopedPtrE()
	{
		return CreateScopedPtr<T>(nullptr);
	}

	template<class T, class... _Ty>
	T* CreateObject(_Ty&& ... params)
	{
		T* M = (T*)FreeAlloc->Allocate(sizeof(T), 8);
		M = new(M) T(std::forward<_Ty>(params)...);
		return M;
	}
}