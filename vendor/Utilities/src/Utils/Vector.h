#pragma once

#include "Types.h"
#include <memory>
#include <type_traits>

#include <functional>
#include <vector>

namespace Utils
{
	//TODO: Align capacity to power of two
	template<class Type>
	class Vector
	{
	public:
		Vector();
		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		Vector(uint32 capacity);
		Vector(uint32 size, const Type& value);

		~Vector();
		void Destroy();

		Type& Push(const Type& value);
		Type& Push(Type&& value) noexcept;
		void Pop();

		Type& Insert(uint32 index, const Type& value);
		Type& Insert(Type* iter, const Type& value);

		//TODO: These are swap and pop, make that more obvious
		void Remove(uint32 index);
		void Remove(Type* iter);

		void Reserve(uint32 capacity);
		void Resize(uint32 size, const Type& value = {});
		void Clear();

		constexpr uint32 ElementSize() const;
		uint32 SizeInBytes() const;
		uint32 Count() const;
		uint32 Capacity() const;
		bool Empty() const;
		bool Full() const;

		const Type* Data() const;
		Type* Data();

		uint32 Find(const Type& val) const;
		uint32 FindIf(std::function<bool(const Type&)> predicate) const;

		Vector& operator=(const Vector& other);
		Vector& operator=(Vector&& other) noexcept;
		const Type& operator[](uint32 index) const;
		Type& operator[](uint32 index);

		const Type& Front() const;
		Type& Front();

		const Type& Back() const;
		Type& Back();

		const Type* begin() const;
		Type* begin();
		const Type* end() const;
		Type* end();

	private:
		void Copy(Type* dst, Type* src, uint32 count);

	private:
		uint32 m_Count = 0;
		uint32 m_Capacity = 0;
		Type* m_Array = nullptr;
	};

	template<class Type> inline uint32 Utils::Vector<Type>::Find(const Type& val) const
	{
		for (uint32 i = 0; i < m_Count; i++)
		{
			if (m_Array[i] == val)
				return i;
		}
		return m_Count;
	}

	template<class Type> uint32 Utils::Vector<Type>::FindIf(std::function<bool(const Type&)> predicate) const
	{
		for (uint32 i = 0; i < m_Count; i++)
		{
			if (predicate(m_Array[i]))
				return i;
		}
		return m_Count;
	}

	template<class Type> inline Vector<Type>::Vector() {}

	template<class Type> inline Vector<Type>::Vector(const Vector<Type>& other) : m_Count{ other.m_Count }
	{
		Reserve(other.m_Capacity);
		Copy(m_Array, other.m_Array, m_Capacity);
	}

	template<class Type> inline Vector<Type>::Vector(Vector<Type>&& other) noexcept : m_Count{ other.m_Count }, m_Capacity{ other.m_Capacity }, m_Array{ other.m_Array }
	{
		other.m_Count = 0;
		other.m_Capacity = 0;
		other.m_Array = nullptr;
	}

	template<class Type> inline Vector<Type>::Vector(uint32 capacity) { Reserve(capacity); }

	template<class Type> inline Vector<Type>::Vector(uint32 count, const Type& value) : m_Count{ count }
	{
		Reserve(count);
		for (Type* t = m_Array, *end = m_Array + m_Count; t != end; ++t) { *t = value; }
	}

	template<class Type> inline Vector<Type>::~Vector()
	{
		Destroy();
	}

	template<class Type> inline void Vector<Type>::Destroy()
	{
		m_Count = 0;
		m_Capacity = 0;

		if (m_Array)
		{
			if constexpr (std::is_default_constructible_v<Type>) { delete[] m_Array; }
			else { free(m_Array); }
		}
		m_Array = nullptr;
	}

	template<class Type> inline Type& Vector<Type>::Push(const Type& value)
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		new (m_Array + m_Count) Type(value);
		return m_Array[m_Count++];
	}

	template<class Type> inline Type& Vector<Type>::Push(Type&& value) noexcept
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		new (m_Array + m_Count) Type(std::move(value));
		return m_Array[m_Count++];
	}

	template<class Type> inline void Vector<Type>::Pop()
	{
		if (m_Count) { --m_Count; }
	}

	template<class Type> inline Type& Vector<Type>::Insert(uint32 index, const Type& value)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif

		if (m_Count == 0 || index == m_Count) { return Push(value); }

		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		Copy(m_Array + index + 1, m_Array + index, (m_Count - index));
		m_Array[index] = value;
		++m_Count;

		return m_Array[index];
	}

	template<class Type> inline Type& Vector<Type>::Insert(Type* iter, const Type& value)
	{
		return Insert(iter - m_Array, value);
	}

	template<class Type> inline void Vector<Type>::Remove(uint32 index)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		m_Array[index] = m_Array[--m_Count];
	}

	template<class Type> inline void Vector<Type>::Remove(Type* iter)
	{
		uint32 index = iter - m_Array;

#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif

		m_Array[index] = m_Array[--m_Count];
	}
	
	template<class Type> inline void Vector<Type>::Reserve(uint32 capacity)
	{
		//TODO: Realloc with custom memory

		if (m_Array)
		{
			Type* temp = m_Array;
			if constexpr (std::is_default_constructible_v<Type>) { m_Array = new Type[capacity]; }
			else { m_Array = (Type*)malloc(capacity * sizeof(Type)); }

			Copy(m_Array, temp, m_Capacity);

			if constexpr (std::is_default_constructible_v<Type>) { delete[] temp; }
			else { free(temp); }
			
		}
		else
		{
			if constexpr (std::is_default_constructible_v<Type>) { m_Array = new Type[capacity]; }
			else { m_Array = (Type*)malloc(capacity * sizeof(Type)); }
		}

		m_Capacity = capacity;
	}

	template<class Type> inline void Vector<Type>::Resize(uint32 count, const Type& value)
	{
		if (count > m_Capacity) { Reserve(count); }

		for (Type* t = m_Array + m_Count, *end = m_Array + count; t != end; ++t) { *t = value; }

		m_Count = count;
	}

	template<class Type> inline void Vector<Type>::Clear() { m_Count = 0; }

	template<class Type> inline constexpr uint32 Vector<Type>::ElementSize() const { return sizeof(Type); }

	template<class Type> inline uint32 Vector<Type>::SizeInBytes() const { return m_Count * sizeof(Type); }

	template<class Type> inline uint32 Vector<Type>::Count() const { return m_Count; }

	template<class Type> inline uint32 Vector<Type>::Capacity() const { return m_Capacity; }

	template<class Type> inline bool Vector<Type>::Empty() const { return m_Count == 0; }

	template<class Type> inline bool Vector<Type>::Full() const { return m_Count == m_Capacity; }

	template<class Type> inline const Type* Vector<Type>::Data() const { return m_Array; }

	template<class Type> inline Type* Vector<Type>::Data() { return m_Array; }

	template<class Type> inline Vector<Type>& Vector<Type>::operator=(const Vector& other)
	{
		Destroy();

		Resize(other.m_Count);
		Copy(m_Array, other.m_Array, m_Capacity);

		return *this;
	}

	template<class Type> inline Vector<Type>& Vector<Type>::operator=(Vector&& other) noexcept
	{
		Destroy();

		m_Count = other.m_Count;
		m_Capacity = other.m_Capacity;
		m_Array = other.m_Array;

		other.m_Count = 0;
		other.m_Capacity = 0;
		other.m_Array = nullptr;

		return *this;
	}

	template<class Type> inline const Type& Vector<Type>::operator[](uint32 index) const
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		return m_Array[index];
	}
	template<class Type> inline Type& Vector<Type>::operator[](uint32 index)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		return m_Array[index];
	}

	template<class Type> inline const Type& Vector<Type>::Front() const { return *m_Array; }

	template<class Type> inline Type& Vector<Type>::Front() { return *m_Array; }

	template<class Type> inline const Type& Vector<Type>::Back() const { return m_Array[m_Count - 1]; }

	template<class Type> inline Type& Vector<Type>::Back() { return m_Array[m_Count - 1]; }

	template<class Type> inline const Type* Vector<Type>::begin() const { return m_Array; }

	template<class Type> inline Type* Vector<Type>::begin() { return m_Array; }

	template<class Type> inline const Type* Vector<Type>::end() const { return m_Array + m_Count; }

	template<class Type> inline Type* Vector<Type>::end() { return m_Array + m_Count; }

	template<class Type> inline void Vector<Type>::Copy(Type* dst, Type* src, uint32 count)
	{
		if (dst > src && dst < (src + count))
		{
			src += count - 1;
			dst += count - 1;

			for (uint32 i = 0; i < count; ++i)
			{
				new (dst--) Type(*src--);
			}
		}
		else
		{
			for (uint32 i = 0; i < count; ++i)
			{
				new (dst++) Type(*src++);
			}
		}
	}
}
