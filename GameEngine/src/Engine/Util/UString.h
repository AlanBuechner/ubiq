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

		int Size();

		void operator=(const UString& str);
		bool operator==(const UString& str);
		void operator+=(const UString& str);
		UString operator+(const UString& str);

		friend std::ostream& operator<<(std::ostream& os, const UString& str);

	private:
		char* m_Data = nullptr;

		static FreeListAllocator* alloc;
	};
}