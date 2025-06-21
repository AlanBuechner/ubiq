#pragma once

#include "Types.h"
#include <memory>
#include <type_traits>
#include <functional>

#include <vector>

namespace Utils
{
	template<class Type>
	class Vector : protected std::vector<Type>
	{
	public:
		using Base = std::vector<Type>;
		using Iterator = typename Base::iterator;
		using ConstIterator = typename Base::const_iterator;

		using Predicate = std::function<bool(const Type&)>;

	public:
		Vector();
		Vector(const Vector& other);
		Vector(Vector&& other) noexcept;
		Vector(uint32 count);
		Vector(uint32 count, const Type& value);
		Vector(std::initializer_list<Type> list);

		~Vector();

		Type& Push(const Type& value);
		Type& Push(Type&& value) noexcept;
		Type Pop();

		Type& Insert(uint32 index, const Type& value);
		Type& Insert(Type* pos, const Type& value);
		Type& Insert(Iterator pos, const Type& value);
		Type& Insert(ConstIterator pos, const Type& value);

		void Remove(uint32 index);
		void Remove(Type* pos);
		void Remove(Iterator pos);
		void Remove(ConstIterator pos);

		void RemoveRange(uint32 start, uint32 end);
		void RemoveRange(Type* start, Type* end);
		void RemoveRange(Iterator start, Iterator end);
		void RemoveRange(ConstIterator start, ConstIterator end);

		void SwapAndPop(uint32 index);
		void SwapAndPop(Type* pos);
		void SwapAndPop(Iterator pos);
		void SwapAndPop(ConstIterator pos);

		void Reserve(uint32 capacity);
		void ReserveMore(uint32 capacity);
		void Resize(uint32 size, const Type& value = {});
		void Clear();

		uint32 Find(const Type& val) const;
		uint32 FindIf(Predicate predicate) const;

		bool Contains(const Type& val) const { return Find(val) != Count(); }

		constexpr uint32 ElementSize() const;
		uint32 SizeInBytes() const;
		uint32 Count() const;
		uint32 Capacity() const;
		bool Empty() const;
		bool Full() const;

		const Type* Data() const;
		Type* Data();

		Vector& operator=(const Vector& other);
		Vector& operator=(Vector&& other) noexcept;
		bool operator==(const Vector<Type>& other) const;
		bool operator!=(const Vector<Type>& other) const;
		const Type& operator[](uint32 index) const;
		Type& operator[](uint32 index);

		const Type& Front() const;
		Type& Front();

		const Type& Back() const;
		Type& Back();

		ConstIterator begin() const;
		Iterator begin();
		ConstIterator end() const;
		Iterator end();

		std::vector<Type>& CastToSTD() { return (std::vector<Type>&) * this; }
	};

	template<class Type> 
	inline Vector<Type>::Vector()
	{}

	template<class Type>
	inline Vector<Type>::Vector(const Vector<Type>& other) :
		std::vector<Type>(other)
	{}

	template<class Type>
	inline Vector<Type>::Vector(Vector<Type>&& other) noexcept :
		std::vector<Type>(other)
	{}

	template<class Type>
	inline Vector<Type>::Vector(uint32 count) :
		std::vector<Type>(count)
	{}

	template<class Type>
	inline Vector<Type>::Vector(uint32 count, const Type& value) :
		std::vector<Type>(count, value)
	{}

	template<class Type>
	inline Vector<Type>::Vector(std::initializer_list<Type> list) :
		std::vector<Type>(list)
	{}

	template<class Type> 
	inline Vector<Type>::~Vector()
	{}




	template<class Type>
	inline Type& Vector<Type>::Push(const Type& value)
	{
		Base::push_back(value);
		return Base::back();
	}

	template<class Type>
	inline Type& Vector<Type>::Push(Type&& value) noexcept
	{
		Base::push_back(value);
		return Base::back();
	}

	template<class Type>
	inline Type Vector<Type>::Pop()
	{
		Type val = Base::back();
		Base::pop_back();
		return val;
	}

	template<class Type>
	inline Type& Vector<Type>::Insert(uint32 index, const Type& value)
	{
		return *Base::insert(Base::begin() + index, value);
	}

	template<class Type>
	inline Type& Vector<Type>::Insert(Type* pos, const Type& value)
	{
		return Insert(Base::begin() - pos, value);
	}

	template<class Type>
	inline Type& Vector<Type>::Insert(Vector<Type>::Iterator pos, const Type& value)
	{
		return *Base::insert(pos, value);
	}

	template<class Type>
	inline Type& Vector<Type>::Insert(Vector<Type>::ConstIterator pos, const Type& value)
	{
		return *Base::insert(pos, value);
	}

	template<class Type> inline void Vector<Type>::Remove(uint32 index) { Base::erase(Base::begin() + index); }
	template<class Type> inline void Vector<Type>::Remove(Type* pos) { Base::erase(std::remove(Base::begin(), Base::end(), pos), Base::end()); }
	template<class Type> inline void Vector<Type>::Remove(Vector<Type>::Iterator pos) { Base::erase(pos); }
	template<class Type> inline void Vector<Type>::Remove(Vector<Type>::ConstIterator pos) { Base::erase(pos); }

	template<class Type> void Vector<Type>::RemoveRange(uint32 start, uint32 end) { Base::erase(Base::Begin() + start, Base::Begin() + end); }
	template<class Type> void Vector<Type>::RemoveRange(Type* start, Type* end) { Base::erase(start, end); }
	template<class Type> void Vector<Type>::RemoveRange(Vector<Type>::Iterator start, Vector<Type>::Iterator end) { Base::erase(start, end); }
	template<class Type> void Vector<Type>::RemoveRange(Vector<Type>::ConstIterator start, Vector<Type>::ConstIterator end) { Base::erase(start, end); }

	template<class Type> inline void Vector<Type>::SwapAndPop(uint32 index) { Base::data()[index] = std::move(Base::back()); Base::pop_back(); }
	template<class Type> inline void Vector<Type>::SwapAndPop(Type* pos) { SwapAndPop(Base::begin() - pos); }
	template<class Type> inline void Vector<Type>::SwapAndPop(Vector<Type>::Iterator pos) { SwapAndPop(Base::begin()-pos); }
	template<class Type> inline void Vector<Type>::SwapAndPop(Vector<Type>::ConstIterator pos) { SwapAndPop(Base::begin() - pos); }

	template<class Type> inline void Vector<Type>::Reserve(uint32 capacity) { Base::reserve(capacity); }
	template<class Type> inline void Vector<Type>::ReserveMore(uint32 capacity) { Base::reserve(Count() + capacity); }

	template<class Type> inline void Vector<Type>::Resize(uint32 count, const Type& value) { Base::resize(count, value); }

	template<class Type>
	inline uint32 Vector<Type>::Find(const Type& val) const
	{
		for (uint32 i = 0; i < Base::size(); i++)
		{
			if (Base::data()[i] == val)
				return i;
		}
		return Base::size();
	}

	template<class Type> uint32 Vector<Type>::FindIf(Predicate predicate) const
	{
		for (uint32 i = 0; i < Base::size(); i++)
		{
			if (predicate(Base::data()[i]))
				return i;
		}
		return Base::size();
	}

	template<class Type> inline void Vector<Type>::Clear() { Base::clear(); }
	template<class Type> inline constexpr uint32 Vector<Type>::ElementSize() const { return sizeof(Type); }
	template<class Type> inline uint32 Vector<Type>::SizeInBytes() const { return Base::size() * sizeof(Type); }
	template<class Type> inline uint32 Vector<Type>::Count() const { return Base::size(); }
	template<class Type> inline uint32 Vector<Type>::Capacity() const { return Base::capacity(); }
	template<class Type> inline bool Vector<Type>::Empty() const { return Base::empty(); }
	template<class Type> inline bool Vector<Type>::Full() const { return Base::size() == Base::capacity(); }
	template<class Type> inline const Type* Vector<Type>::Data() const { return Base::data(); }
	template<class Type> inline Type* Vector<Type>::Data() { return Base::data(); }

	template<class Type> inline Vector<Type>& Vector<Type>::operator=(const Vector& other)
	{
		Base::operator=(other);
		return *this;
	}

	template<class Type>
	inline Vector<Type>& Vector<Type>::operator=(Vector&& other) noexcept
	{
		Base::operator=(other);
		return *this;
	}

	template<class Type>
	inline bool Vector<Type>::operator==(const Vector<Type>& other) const
	{
		for (uint32 i = 0; i < Base::size(); i++)
		{
			if (Base::operator[](i) != other[i])
				return false;
		}
		return true;
	}

	template<class Type>
	inline bool Vector<Type>::operator!=(const Vector<Type>& other) const
	{
		return !((*this) == other);
	}

	template<class Type>
	inline const Type& Vector<Type>::operator[](uint32 index) const
	{
		return Base::operator[](index);
	}
	template<class Type>
	inline Type& Vector<Type>::operator[](uint32 index)
	{
		return Base::operator[](index);
	}

	template<class Type> inline const	Type& Vector<Type>::Front() const	{ return Base::front(); }
	template<class Type> inline			Type& Vector<Type>::Front()			{ return Base::front(); }

	template<class Type> inline const	Type& Vector<Type>::Back() const	{ return Base::back(); }
	template<class Type> inline			Type& Vector<Type>::Back()			{ return Base::back(); }

	template<class Type> inline	typename Vector<Type>::ConstIterator Vector<Type>::begin() const	{ return Base::begin(); }
	template<class Type> inline	typename Vector<Type>::Iterator Vector<Type>::begin()			{ return Base::begin(); }

	template<class Type> inline typename Vector<Type>::ConstIterator Vector<Type>::end() const	{ return Base::end(); }
	template<class Type> inline typename Vector<Type>::Iterator Vector<Type>::end()				{ return Base::end(); }
}
