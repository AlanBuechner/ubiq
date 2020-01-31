#pragma once
#include <iostream>

namespace Engine
{
	class FreeListAllocator;
	class UStringView;
	class IFStream;
}

namespace Engine
{
	class UString
	{
	public:
		UString();
		UString(const char* str);
		UString(int num);
		UString(const UString& str);
		~UString();

		const size_t Size() const;
		char* RawString() const;
		char* Begin() const;
		char* End() const;

		UString SubString(unsigned int start, unsigned int end);

		int Find(const UString& match, const unsigned start = 0);
		int Find(const char match, const unsigned start = 0);

		void operator=(const UString& str);
		void operator=(const char* str);
		void operator=(const int num);
		bool operator==(const UString& str);
		bool operator==(const char* str);
		void operator+=(const UString& str);
		void operator+=(const char* str);
		void operator+=(const int num);
		UString operator+(const UString& str) const;
		UString operator+(const char* str) const;
		UString operator+(const int num) const;
		char& operator[](size_t i) const;

		// cast
		operator char(); // returns the first char in the string

		friend std::ostream& operator<<(std::ostream& os, const UString& str);

		static FreeListAllocator* s_UStringAllocator;
	private:
		void Resize(size_t size);

		void CopyOver(const char* data);

		void* Allocate(size_t size) const;
		void Deallocate(void* p) const;

		static size_t IntLength(int i);

		char* m_Data = nullptr;

		size_t m_Size = 0;


		friend UStringView;
		friend IFStream;
	};

	class UStringView
	{
	public:
		UStringView(UString& String, int start, int end);

		char& operator[](size_t i) const;

		friend std::ostream& operator<<(std::ostream& os, const UStringView& str);

	private:
		const UString* m_String;
		size_t m_Size;
		int m_Start;
		int m_End;
	};
}