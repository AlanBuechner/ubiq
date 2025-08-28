#pragma once
#include "Types.h"

#include <string>

namespace Utils
{
	template<typename Type>
	class StringBase : protected std::basic_string<Type>
	{
	public:
		using Base = std::basic_string<Type>;
		using Iterator = typename Base::iterator;
		using ConstIterator = typename Base::const_iterator;

	public:

		StringBase();
		StringBase(const StringBase& str);
		StringBase(const StringBase& str, uint32 pos, uint32 len = Base::npos);
		StringBase(const char* s);
		StringBase(const char* s, uint32 n);
		StringBase(uint32 n, char c);
		StringBase(std::initializer_list<char> il);
		StringBase(StringBase&& str) noexcept;

		~StringBase();

		Type& Push(const Type& value);
		Type& Push(Type&& value) noexcept;
		void Pop();

		Type& Insert(uint32 index, const StringBase<Type>& value);

		void Remove(uint32 index);
		void RemoveRange(uint32 start, uint32 end);
		
		void Reserve(uint32 capacity);
		void ReserveMore(uint32 capacity);
		void Resize(uint32 count);
		void Resize(uint32 count, char c);
		void Clear();
		
		constexpr uint32 ElementSize() const;
		uint32 SizeInBytes() const;
		uint32 Count() const;
		uint32 Capacity() const;
		bool Empty() const;
		bool Full() const;

		Type* Data();
		const Type* Data() const;

		StringBase& operator=(const StringBase& other);
		StringBase& operator=(StringBase&& other) noexcept;
		bool operator==(const StringBase<Type>& other) const;
		bool operator!=(const StringBase<Type>& other) const;
		const Type& operator[](uint32 index) const;
		Type& operator[](uint32 index);
		
		StringBase& operator+=(const StringBase& str);
		StringBase& operator+=(const char* str);
		StringBase& operator+=(char c);

		Type& At(uint32 i);
		const Type& At(uint32 i) const;

		Type& Front();
		const Type& Front() const;

		Type& Back();
		const Type& Back() const;

		ConstIterator begin() const;
		Iterator begin();
		ConstIterator end() const;
		Iterator end();

		std::basic_string<Type>& CastToSTD() { return (std::basic_string<Type>&) * this; }

	};

	template<typename Type>
	StringBase<Type>::StringBase()
	{}

	template<typename Type>
	StringBase<Type>::StringBase(const StringBase& str) :
		Base(str)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(const StringBase& str, uint32 pos, uint32 len) :
		Base(str, pos, len)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(const char* s) :
		Base(s)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(const char* s, uint32 n) :
		Base(s, n)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(uint32 n, char c) :
		Base(n, c)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(std::initializer_list<char> il) :
		Base(il)
	{}

	template<typename Type>
	StringBase<Type>::StringBase(StringBase&& str) noexcept :
		Base(str)
	{}

	template<typename Type>
	StringBase<Type>::~StringBase()
	{}


	template<typename Type> Type& StringBase<Type>::Push(const Type& value) { Base::push_back(value); }
	template<typename Type> Type& StringBase<Type>::Push(Type&& value) noexcept { return Base::push_back(value); }
	template<typename Type> void StringBase<Type>::Pop() { Base::pop_back(); }
	template<typename Type> Type& StringBase<Type>::Insert(uint32 index, const StringBase<Type>& value) { return Base::insert(index, value); }
	template<typename Type> void StringBase<Type>::Remove(uint32 index) { Base::erase(index); }
	template<typename Type> void StringBase<Type>::RemoveRange(uint32 start, uint32 end) { Base::erase(start, (end-start)); }

	template<typename Type> void StringBase<Type>::Reserve(uint32 capacity) { Base::reserve(capacity); }
	template<typename Type> void StringBase<Type>::ReserveMore(uint32 capacity) { Reserve(Count() + capacity); }
	template<typename Type> void StringBase<Type>::Resize(uint32 count) { Base::resize(count); }
	template<typename Type> void StringBase<Type>::Resize(uint32 count, char c) { Base::resize(count, c); }

	template<typename Type> void StringBase<Type>::Clear() { return Base::clear(); }

	template<typename Type> constexpr uint32 StringBase<Type>::ElementSize() const { return sizeof(Type); }
	template<typename Type> uint32 StringBase<Type>::SizeInBytes() const { return Count() * ElementSize(); }
	template<typename Type> uint32 StringBase<Type>::Count() const { return Base::size(); }
	template<typename Type> uint32 StringBase<Type>::Capacity() const { return Base::capacity(); }

	template<typename Type> bool StringBase<Type>::Empty() const { return Base::empty(); }
	template<typename Type> bool StringBase<Type>::Full() const { return Count() == Capacity(); }

	template<typename Type> Type* StringBase<Type>::Data() { return Base::data(); }
	template<typename Type> const Type* StringBase<Type>::Data() const { return Base::data(); }


	template<typename Type>
	StringBase<Type>::Iterator StringBase<Type>::end()
	{

	}

	template<typename Type>
	StringBase<Type>::ConstIterator StringBase<Type>::end() const
	{

	}

	template<typename Type>
	StringBase<Type>::Iterator StringBase<Type>::begin()
	{

	}

	template<typename Type>
	StringBase<Type>::ConstIterator StringBase<Type>::begin() const
	{

	}

	template<typename Type>
	Type& StringBase<Type>::Back()
	{

	}

	template<typename Type>
	const Type& StringBase<Type>::Back() const
	{

	}

	template<typename Type>
	Type& StringBase<Type>::Front()
	{

	}

	template<typename Type>
	const Type& StringBase<Type>::Front() const
	{

	}

	template<typename Type>
	const Type& StringBase<Type>::At(uint32 i) const
	{

	}

	template<typename Type>
	Type& StringBase<Type>::At(uint32 i)
	{

	}

	template<typename Type>
	StringBase<Type>& StringBase<Type>::operator+=(char c)
	{

	}

	template<typename Type>
	StringBase<Type>& StringBase<Type>::operator+=(const char* str)
	{

	}

	template<typename Type>
	StringBase<Type>& StringBase<Type>::operator+=(const StringBase& str)
	{

	}

	template<typename Type>
	Type& StringBase<Type>::operator[](uint32 index)
	{

	}

	template<typename Type>
	const Type& StringBase<Type>::operator[](uint32 index) const
	{

	}

	template<typename Type>
	bool StringBase<Type>::operator!=(const StringBase<Type>& other) const
	{

	}

	template<typename Type>
	bool StringBase<Type>::operator==(const StringBase<Type>& other) const
	{

	}

	template<typename Type>
	StringBase<Type>& StringBase<Type>::operator=(StringBase&& other) noexcept
	{

	}

	template<typename Type>
	StringBase<Type>& StringBase<Type>::operator=(const StringBase& other)
	{

	}


	using String = StringBase<char>;
	using WString = StringBase<wchar_t>;
}
