#pragma once
#include <iostream>

namespace Engine
{
	class FreeListAllocator;
	class UStringView;
}

namespace Engine
{
	class UString
	{
	public:
		UString();
		UString(const char* str);
		UString(int size);
		UString(const UString& str);
		~UString();

		const size_t Size() const;
		char* RawString() const;
		char* Begin() const;
		char* End() const;

		UString SubString(unsigned int start, unsigned int end);

		bool Find(const UString& match);

		void operator=(const UString& str);
		void operator=(const char* str);
		bool operator==(const UString& str);
		bool operator==(const char* str);
		void operator+=(const UString& str);
		void operator+=(const char* str);
		UString operator+(const UString& str);
		UString operator+(const char* str);
		char& operator[](size_t i) const;

		friend std::ostream& operator<<(std::ostream& os, const UString& str);

		static FreeListAllocator* s_UStringAllocator;
	private:
		void Resize(size_t size);

		void CopyOver(const char* data);

		char* m_Data = nullptr;

		size_t m_Size = 0;


		friend UStringView;
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