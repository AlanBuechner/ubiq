#pragma once
#include "Engine/Core/Memory/FreeListAllocator.h"
#include <ostream>

namespace Engine
{
	static FreeListAllocator* UArrayAlloc = new FreeListAllocator(1000000000, FreeListAllocator::FindFirst, 8);

	template<class T>
	class UArray
	{
	public:
		UArray();

		UArray(size_t size);

		UArray(std::initializer_list<T> data);

		UArray(UArray<T>& other);

		~UArray();

		T* Begin();

		T* End();

		size_t Size(); // size of the array in bytes

		size_t Length(); // the number of elements in the array

		void ReSize(size_t size);

		void PushBack(const T& data);

		T PopBack();

		void Clear();

		T& operator[](size_t i);

		void operator=(const UArray<T>& other);
		bool operator==(const UArray<T>& other);

	private:
		T* m_Data = nullptr;
		size_t m_Size;
	};

	template<class T>
	inline UArray<T>::UArray()
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(0);
		m_Size = 0;
	}

	template<class T>
	inline UArray<T>::UArray(size_t size)
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(T) * size);
		m_Size = size;
	}

	template<class T>
	inline UArray<T>::UArray(std::initializer_list<T> data)
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(data.begin()));
		m_Size = data.size();
		for (int i = 0; i < Length(); i++)
		{
			m_Data[i] = data.begin()[i];
		}
	}

	template<class T>
	inline UArray<T>::UArray(UArray<T>& other)
	{
		if (m_Data != nullptr)
			UArrayAlloc->Deallocate(m_Data);
		size_t size = other.Size();
		size_t length = other.Length();
		m_Data = (T*)UArrayAlloc->Allocate(size);
		for (int i = 0; i < length; i++)
		{
			m_Data[i] = other.m_Data[i];
		}
		m_Size = length;
	}

	template<class T>
	inline UArray<T>::~UArray()
	{
		UArrayAlloc->Deallocate(m_Data);
	}

	template<class T>
	inline T* UArray<T>::Begin()
	{
		return m_Data;
	}

	template<class T>
	inline T* UArray<T>::End()
	{
		return return m_Data + Size();
	}

	template<class T>
	inline size_t UArray<T>::Size()
	{
		return sizeof(m_Data);
	}

	template<class T>
	inline size_t UArray<T>::Length()
	{
		return m_Size;
	}

	template<class T>
	inline void UArray<T>::ReSize(size_t size)
	{
		T* temp = m_Data;
		m_Data = (T*)UArrayAlloc->Allocate(sizeof(T) * size);
		m_Size = size;
		for (int i = 0; i < Length(); i++)
		{
			m_Data[i] = temp[i];
		}
		UArrayAlloc->Deallocate(temp);
	}

	template<class T>
	inline void UArray<T>::PushBack(const T& data)
	{
		size_t length = Length();
		ReSize(length +1);
		m_Data[length] = data;
	}

	template<class T>
	inline T UArray<T>::PopBack()
	{
		size_t newSize = Length() - 1;
		T& toRetern = m_Data[newSize];
		ReSize(newSize);
		return toRetern;
	}

	template<class T>
	inline void UArray<T>::Clear()
	{
		ReSize(0);
	}

	template<class T>
	inline T& UArray<T>::operator[](size_t i)
	{
		return m_Data[i];
	}

	template<class T>
	inline void UArray<T>::operator=(const UArray<T>& other)
	{
		ReSize(other.Length());
		for (int i = 0; i < Length(); i++)
		{
			m_Data[i] = other.m_Data[i];
		}
	}

	template<class T>
	inline bool UArray<T>::operator==(const UArray<T>& other)
	{
		if (Size() == sizeof(other.m_Data))
		{
			for (int i = 0; i < Length(); i++)
			{
				if (m_Data[i] != other.m_Data[i])
					return false;
			}
			return true;
		}
		return false;
	}

}