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
		template <class... Parameters>
		Type& Emplace(Parameters&&... parameters) noexcept;
		void Pop();
		void Pop(Type& value);

		Type& Insert(uint32 index, const Type& value);
		Type& Insert(uint32 index, Type&& value);

		//TODO: These are swap and pop, make that more obvious
		void Remove(uint32 index);
		void Remove(uint32 index, Type& value);
		void Remove(Type* iter);
		void Remove(Type* iter, Type& value);

		void Reserve(uint32 capacity);
		void Resize(uint32 count);
		void Resize(uint32 count, const Type& value);
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
		template <class... Parameters>
		Type& Emplace_Internal(Type* dst, Parameters&&... parameters) noexcept;
		Type* CopyValues(Type* dst, const Type* src, uint32_t count);
		Type* MoveValues(Type* dst, Type* src, uint32_t count);

	private:
		uint32 m_Count = 0;
		uint32 m_Capacity = 0;
		Type* m_Array = nullptr;
	};

	template<class Type> inline Vector<Type>::Vector() {}

	template<class Type> inline Vector<Type>::Vector(const Vector<Type>& other) : m_Count{ other.m_Count }
	{
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
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
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
		Reserve(m_Count);
		for (Type* it = m_Array, *end = m_Array + m_Count; it != end; ++it) { Emplace_Internal(it, value); }
	}

	template<class Type> inline Vector<Type>::Vector(std::initializer_list<Type> list) : m_Count{ (uint32)list.size() }
	{
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
		Reserve(m_Count);
		CopyValues(m_Array, list.begin(), m_Count);
	}

	template<class Type> inline Vector<Type>::~Vector()
	{
		Destroy();
	}

	template<class Type> inline void Vector<Type>::Destroy()
	{
		if (m_Array)
		{
			if constexpr (UseDestructor) {
				for (uint32 i = 0; i < m_Count; i++)
					(m_Array + i)->~Type();
			}
			free(m_Array);
		}

		m_Count = 0;
		m_Capacity = 0;
		m_Array = nullptr;
	}

	template<class Type> inline Type& Vector<Type>::Push(const Type& value)
	{
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		return Emplace_Internal(m_Array + m_Count++, value);
	}

	template<class Type> inline Type& Vector<Type>::Push(Type&& value) noexcept
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		return Emplace_Internal(m_Array + m_Count++, std::move(value));
	}

	template<class Type>
	template <class... Parameters>
	inline Type& Vector<Type>::Emplace(Parameters&&... parameters) noexcept
	{
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		return Emplace_Internal(m_Array + m_Count++, std::forward<Parameters>(parameters)...);
	}

	template<class Type> inline void Vector<Type>::Pop()
	{
		if (m_Count) {
			if constexpr (UseDestructor)
				(m_Array + m_Count - 1)->~Type();
			--m_Count;
		}
	}

	template<class Type>
	inline void Vector<Type>::Pop(Type& value)
	{
		if (m_Count)
		{
			Emplace_Internal(&value, std::move(m_Array[--m_Count]));
		}
	}

	template<class Type> inline Type& Vector<Type>::Insert(uint32 index, const Type& value)
	{
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		MoveValues(m_Array + index + 1, m_Array + index, (m_Count - index));
		++m_Count;
		return Emplace_Internal(m_Array + index, value);
	}

	template<class Type> inline Type& Vector<Type>::Insert(uint32 index, Type&& value)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		if (m_Count == m_Capacity) { Reserve((m_Capacity + 1) * 2); }

		MoveValues(m_Array + index + 1, m_Array + index, (m_Count - index));
		++m_Count;
		return Emplace_Internal(m_Array + index, std::move(value));
	}

	template<class Type> inline void Vector<Type>::Remove(uint32 index)
	{
		if constexpr (std::is_destructible_v<Type>) { (m_Array + index)->~Type(); }
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		Emplace_Internal(m_Array + index, std::move(m_Array[--m_Count]));
	}

	template<class Type> inline void Vector<Type>::Remove(uint32 index, Type& value)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		Emplace_Internal(value, std::move(m_Array[index]));
		Emplace_Internal(m_Array + index, std::move(m_Array[--m_Count]));
	}

	template<class Type> inline void Vector<Type>::Remove(Type* iter)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		uint32 index = iter - m_Array;
		if constexpr (std::is_destructible_v<Type>) { (m_Array + index)->~Type(); }

		Emplace_Internal(m_Array + index, std::move(m_Array[--m_Count]));
	}

	template<class Type> inline void Vector<Type>::Remove(Type* iter, Type& value)
	{
#if defined(DEBUG)
		//TODO: Assert index in bounds
#endif
		uint32 index = iter - m_Array;
		Emplace_Internal(value, std::move(m_Array[index]));
		Emplace_Internal(m_Array + index, std::move(m_Array[--m_Count]));
	}
	
	template<class Type> inline void Vector<Type>::Reserve(uint32 capacity)
	{
		//TODO: Realloc with custom memory

		if (m_Array)
		{
			// create new array
			Type* temp = m_Array;
			m_Array = (Type*)malloc(capacity * sizeof(Type));

			// move data from old array to new array
			// move takes care of destruction
			MoveValues(m_Array, temp, m_Count);

			free(temp);
		}
		else
		{
			m_Array = (Type*)malloc(capacity * sizeof(Type));
		}

		m_Capacity = capacity;
	}


	template<class Type>
	inline void Utils::Vector<Type>::Resize(uint32 count)
	{
		// call default constructor when no value is given
		if (count > m_Capacity)
		{
			Reserve(count);

			if constexpr (std::is_default_constructible_v<Type>)
			{
				for (uint32 i = m_Count; i < count; i++)
					new (m_Array + i) Type();
			}
			
		}

		m_Count = count;
	}


	template<class Type> 
	inline void Vector<Type>::Resize(uint32 count, const Type& value)
	{
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");

		if (count > m_Capacity) 
		{
			Reserve(count);
			for (uint32 i = m_Count; i < count; i++) 
				Emplace_Internal(m_Array + i, value);
		}

		m_Count = count;
	}

	template<class Type> inline void Vector<Type>::Clear()
	{
		if (m_Array)
		{
			if constexpr (UseDestructor) {
				for (uint32 i = 0; i < m_Count; i++)
					(m_Array + i)->~Type();
			}
		}

		m_Count = 0;
	}

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
		static_assert(std::is_copy_constructible_v<Type>, "Type Must Be Copyable!");
		Destroy();

		Reserve(other.m_Count);
		m_Count = other.m_Count;

		CopyValues(m_Array, other.m_Array, m_Count);

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

	template<class Type>
	template <class... Parameters>
	inline Type& Vector<Type>::Emplace_Internal(Type* dst, Parameters&&... parameters) noexcept
	{
		return *(new (dst) Type(std::forward<Parameters>(parameters)...));
	}

	template <class Type>
	inline Type* Vector<Type>::CopyValues(Type* dst, const Type* src, uint32_t count)
	{
		if constexpr (std::is_class_v<Type> || std::is_union_v<Type>) //Only emplace non primitive types
		{
			if (dst > src && dst < src + count) //Reverse Copy if dst would overrite src
			{
				Type* rDst = dst + count - 1;
				const Type* rSrc = src + count - 1;

				//TODO: look into unrolling
				for (uint32_t i = 0; i < count; ++i)
					Emplace_Internal(rDst--, *rSrc--);
			}
			else
			{
				for (uint32_t i = 0; i < count; ++i)
					Emplace_Internal(dst++, *src++);
			}

			return dst;
		}
		else
		{
			return (Type*)memcpy(dst, src, sizeof(Type) * count);
		}
	}

	template <class Type>
	inline Type* Vector<Type>::MoveValues(Type* dst, Type* src, uint32_t count)
	{
		if constexpr (std::is_class_v<Type> || std::is_union_v<Type>) //Only emplace non primitive types
		{
			if (dst > src && dst < src + count) //Reverse Copy if dst would overrite src
			{
				Type* rDst = dst + count - 1;
				Type* rSrc = src + count - 1;

				//TODO: look into unrolling
				for (uint32_t i = 0; i < count; ++i)
				{
					Emplace_Internal(rDst--, std::move(*rSrc));
					if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { rSrc->~Type(); }
					--rSrc;
				}
			}
			else
			{
				for (uint32_t i = 0; i < count; ++i)
				{
					Emplace_Internal(dst++, std::move(*src));
					if (!std::is_move_constructible_v<Type> && std::is_destructible_v<Type>) { src->~Type(); }
					++src;
				}
			}

			return dst;
		}
		else
		{
			return (Type*)memcpy(dst, src, sizeof(Type) * count);
		}
	}
}
