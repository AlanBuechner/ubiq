#pragma once
#include "Engine/Core/Core.h"
#include "ComponentPool.h"
#include "ComponentPool.h"

namespace Engine
{

	template<class T>
	class ComponentItorator
	{
	public:
		ComponentItorator(SizeComponentPool<sizeof(T)>* pool, uint32 index) :
			m_Pool(pool), m_Index(index)
		{}

		ComponentItorator& operator++()
		{
			m_Index++;
			return *this;
		}

		ComponentItorator operator++(int)
		{
			ComponentItorator<T> iterator = *this;
			++(*this);
			return iterator;
		}

		ComponentItorator& operator--()
		{
			m_Index--;
			return *this;
		}

		ComponentItorator operator--(int)
		{
			ComponentItorator<T> iterator = *this;
			--(*this);
			return iterator;
		}

		T& operator[](int index)
		{
			return	*(T*)m_Pool->GetComponentMemory(m_Pool->GetUsedSlots()[index]);
		}

		T* operator->()
		{
			return (T*)m_Pool->GetComponentMemory(m_Pool->GetUsedSlots()[m_Index]);
		}

		T& operator*()
		{
			return (*this)[m_Index];
		}

		bool operator==(ComponentItorator& other)
		{
			return m_Index == other->m_Index;
		}

		bool operator!=(ComponentItorator& other)
		{
			return m_Index != other.m_Index;
		}


	private:
		uint32 m_Index = 0;
		SizeComponentPool<sizeof(T)>* m_Pool;
	};

	template<class T>
	class ComponentView
	{
	public:
		using iterator = ComponentItorator<T>;

		ComponentView(SizeComponentPool<sizeof(T)>* pool) :
			m_Pool(pool)
		{}

		iterator begin()
		{
			return iterator(m_Pool, 0);
		}

		iterator end()
		{
			return iterator(m_Pool, (uint32)m_Pool->GetUsedSlots().size());
		}

	private:
		SizeComponentPool<sizeof(T)>* m_Pool;
		
	};
}