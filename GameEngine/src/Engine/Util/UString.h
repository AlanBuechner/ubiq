#pragma once
#include <iostream>

namespace Engine
{
	class FreeListAllocator;
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

		const size_t Size();
		char* RawString();
		char* Begin();
		char* End();

		void operator=(const UString& str);
		void operator=(const char* str);
		bool operator==(const UString& str);
		bool operator==(const char* str);
		void operator+=(const UString& str);
		void operator+=(const char* str);
		UString operator+(const UString& str);
		UString operator+(const char* str);
		char& operator[](size_t i);

		friend std::ostream& operator<<(std::ostream& os, const UString& str);

	public:
		char* m_Data = nullptr;

		static FreeListAllocator* s_UStringAllocator;
	};
}