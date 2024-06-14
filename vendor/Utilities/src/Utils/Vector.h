#pragma once

#include "Types.h"

#include <memory>
#include <type_traits>
#include <functional>
#include <initializer_list>

namespace Utils
{
	//TODO: Align capacity to power of two
	template<class Type>
	class Vector
	{
		using Predicate = std::function<bool(const Type&)>;
		using Compare = std::function<bool(const Type&, const Type&)>;

		static constexpr inline bool UseNew = std::is_default_constructible_v<Type>;
		static constexpr inline bool UseDestructor = std::is_destructible_v<Type>;

	public:
		Vector();
		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		Vector(uint32 capacity);
		Vector(uint32 size, const Type& value);
		Vector(std::initializer_list<Type> list);

		~Vector();
		void Destroy();

		Type& Push(const Type& value);
		Type& Push(Type&& value) noexcept;
		void Pop();

		Type& Insert(uint32 index, const Type& value);

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
		uint32 FindIf(Predicate predicate) const;

		Vector& operator=(const Vector& other);
		Vector& operator=(Vector&& other) noexcept;
		bool operator==(const Vector& other) const;
		bool operator!=(const Vector& other) const;
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
		Type* CopyValue(Type* dst, const Type& value);
		Type* MoveValue(Type* dst, Type&& value) noexcept;
		Type* CopyValues(Type* dst, Type* src, uint32_t count);
		Type* MoveValues(Type* dst, Type* src, uint32_t count);

	private:
		uint32 m_Count = 0;
		uint32 m_Capacity = 0;
		Type* m_Array = nullptr;
	};

	template<class Type> inline Vector<Type>::Vector() {}

	template<class Type> inline Vector<Type>::Vector(const Vector<Type>& other) : m_Count{ other.m_Count }
	{
		Reserve(m_Count);
		CopyValues(m_Array, other.m_Array, m_Count);
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
		Reserve(m_Count);
		for (Type* t = m_Array, *end = m_Array + m_Count; t != end; ++t) { CopyValue(t, value); }
	}

	template<class Type> inline Vector<Type>::Vector(std::initializer_list<Type> list) : m_Count{ (uint32)list.size() }
	{
		Reserve(m_Count);

		Type* arr = m_Array;
		for (const Type* it = list.begin(); it != list.end(); ++it, ++arr)
		{
			CopyValue(arr, *it);
		}
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
			if constexpr (UseNew) { delete[] m_Array; }
			else
			{
				if constexpr (UseDestructor) { for (Type* t = m_Array, *end = m_Array + m_Count; t != end; ++t) { t->~Type(); } }
				free(m_Array);
			}
		}

		m_Array = nullptr;
	}

	template<class Type> inline Type& Vector<Type>::Push(const Type& value)
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		return *CopyValue(m_Array + m_Count++, value);
	}

	template<class Type> inline Type& Vector<Type>::Push(Type&& value) noexcept
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		return *MoveValue(m_Array + m_Count++, std::move(value));
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

		MoveValues(m_Array + index + 1, m_Array + index, (m_Count - index));
		++m_Count;
		return *CopyValue(m_Array + index, value);
	}

	template<class Type> inline void Vector<Type>::Remove(uint32 index)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		MoveValue(m_Array + index, std::move(m_Array[--m_Count]));
	}

	template<class Type> inline void Vector<Type>::Remove(Type* iter)
	{
		uint32 index = iter - m_Array;

#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif

		MoveValue(m_Array + index, std::move(m_Array[--m_Count]));
	}
	
	template<class Type> inline void Vector<Type>::Reserve(uint32 capacity)
	{
		//TODO: Realloc with custom memory

		if (m_Array)
		{
			Type* temp = m_Array;
			if constexpr (UseNew) { m_Array = new Type[capacity]; }
			else { m_Array = (Type*)malloc(capacity * sizeof(Type)); }

			MoveValues(m_Array, temp, m_Capacity);

			if constexpr (UseNew) { delete[] temp; }
			else
			{
				if constexpr (UseDestructor) { for (Type* t = m_Array, *end = m_Array + m_Count; t != end; ++t) { t->~Type(); } }
				free(temp);
			}
			
		}
		else
		{
			if constexpr (UseNew) { m_Array = new Type[capacity]; }
			else { m_Array = (Type*)malloc(capacity * sizeof(Type)); }
		}

		m_Capacity = capacity;
	}

	template<class Type> inline void Vector<Type>::Resize(uint32 count, const Type& value)
	{
		if (count > m_Capacity) 
		{
			Reserve(count); 
			for (Type* t = m_Array + m_Count, *end = m_Array + count; t != end; ++t) { CopyValue(t, value); }
		}

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

	template<class Type> inline uint32 Vector<Type>::Find(const Type& val) const
	{
		for (uint32 i = 0; i < m_Count; i++)
		{
			if (m_Array[i] == val)
				return i;
		}
		return m_Count;
	}

	template<class Type> uint32 Vector<Type>::FindIf(Predicate predicate) const
	{
		for (uint32 i = 0; i < m_Count; i++)
		{
			if (predicate(m_Array[i]))
				return i;
		}
		return m_Count;
	}

	template<class Type> inline Vector<Type>& Vector<Type>::operator=(const Vector& other)
	{
		Destroy();

		Resize(other.m_Count);
		CopyValues(m_Array, other.m_Array, m_Capacity);

		return *this;
	}

	template<class Type> inline Vector<Type>& Vector<Type>::operator=(Vector<Type>&& other) noexcept
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

	template<class Type> inline bool Vector<Type>::operator==(const Vector<Type>& other) const
	{
		if (this == &other) { return true; }

		if (m_Count != other.m_Count) { return false; }

		for (const Type* it0 = m_Array, *it1 = other.m_Array, *end = m_Array + m_Count; it0 != end; ++it0, ++it1)
		{
			if (*it0 != *it1) { return false; }
		}

		return true;
	}

	template<class Type> inline bool Vector<Type>::operator!=(const Vector<Type>& other) const
	{
		if (this == &other) { return false; }

		if (m_Count != other.m_Count) { return true; }

		for (const Type* it0 = m_Array, *it1 = other.m_Array, *end = m_Array + m_Count; it0 != end; ++it0, ++it1)
		{
			if (*it0 != *it1) { return true; }
		}

		return false;
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

	template <class Type>
	inline Type* Vector<Type>::CopyValue(Type* dst, const Type& value)
	{
		if constexpr (std::is_copy_constructible_v<Type>)
		{
			new (dst) Type(value);
		}
		else if constexpr (std::is_copy_assignable_v<Type>)
		{
			*dst = value;
		}
		else
		{
			memcpy(dst, &value, sizeof(Type));
		}

		return dst;
	}

	template <class Type>
	inline Type* Vector<Type>::MoveValue(Type* dst, Type&& value) noexcept
	{
		if constexpr (std::is_move_constructible_v<Type>)
		{
			new (dst) Type(std::move(value));
		}
		else if constexpr (std::is_move_assignable_v<Type>)
		{
			*dst = std::move(value);
		}
		else
		{
			memcpy(dst, &value, sizeof(Type));
			if constexpr (std::is_destructible_v<Type>) { value.~Type(); }
		}

		return dst;
	}

	template <class Type>
	inline Type* Vector<Type>::CopyValues(Type* dst, Type* src, uint32_t count)
	{
		if (dst > src && dst < src + count) //Reverse Copy
		{
			Type* rDst = dst + count - 1;
			Type* rSrc = src + count - 1;

			//TODO: look into unrolling
			for (uint32_t i = 0; i < count; ++i)
			{
				CopyValue(rDst--, *rSrc--);
			}
		}
		else
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				CopyValue(dst++, *src++);
			}
		}

		return dst;
	}

	template <class Type>
	inline Type* Vector<Type>::MoveValues(Type* dst, Type* src, uint32_t count)
	{
		if (dst > src && dst < src + count) //Reverse Copy
		{
			Type* rDst = dst + count - 1;
			Type* rSrc = src + count - 1;

			//TODO: look into unrolling
			for (uint32_t i = 0; i < count; ++i)
			{
				MoveValue(rDst--, std::move(*rSrc--));
			}
		}
		else
		{
			for (uint32_t i = 0; i < count; ++i)
			{
				MoveValue(dst++, std::move(*src++));
			}
		}

		return dst;
	}
}
