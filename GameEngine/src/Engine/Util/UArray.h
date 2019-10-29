#pragma once
#include "Engine/Core/Memory/FreeListAllocator.h"
#include <ostream>

namespace Engine
{
	static FreeListAllocator* UArrayAlloc = new FreeListAllocator(1000000000, FreeListAllocator::FindFirst);

	template<class T>
	class UArray
	{
	public:
		UArray();

		UArray(size_t size);

		UArray(std::initializer_list<T> data);

		size_t Size(); // size of the array in bytes

		int Length(); // the number of elements in the array

		void ReSize(size_t size);

		void PushBack(const T& data);

		T& operator[](size_t i);

	private:
		T* m_Data = nullptr;
		size_t m_Size;
	};

	template<class T>
	inline UArray<T>::UArray()
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(0, 8);
		m_Size = 0;
	}

	template<class T>
	inline UArray<T>::UArray(size_t size)
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(T) * size, 8);
		m_Size = size;
	}

	template<class T>
	inline UArray<T>::UArray(std::initializer_list<T> data)
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(data.begin()), 8);
		m_Size = sizeof(data.begin()) / sizeof(T);
		for (int i = 0; i < Length(); i++)
		{
			m_Data[i] = data.begin()[i];
		}
	}

	template<class T>
	inline size_t UArray<T>::Size()
	{
		return sizeof(m_Data);
	}

	template<class T>
	inline int UArray<T>::Length()
	{
		return m_Size;
	}

	template<class T>
	inline void UArray<T>::ReSize(size_t size)
	{
		T* temp = m_Data;
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(T) * size, 8);
		m_Size = size;
		for (int i = 0; i < Length(); i++)
		{
			m_Data[i] = temp[i];
		}
		if (temp != nullptr)
			UArrayAlloc->Deallocate(temp);
	}

	template<class T>
	inline void UArray<T>::PushBack(const T& data)
	{
		int lenght = Length();
		ReSize(lenght +1);
		m_Data[lenght] = data;
	}

	template<class T>
	inline T& UArray<T>::operator[](size_t i)
	{
		return m_Data[i];
	}

}