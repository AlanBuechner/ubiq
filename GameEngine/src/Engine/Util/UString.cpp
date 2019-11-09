#include "pch.h"
#include "UString.h"
#include "Engine/Core/Memory/FreeListAllocator.h"

namespace Engine
{
	FreeListAllocator* UString::s_UStringAllocator = new FreeListAllocator(sizeof(char) * 10000, FreeListAllocator::FindFirst);

	UString::UString()
	{
		m_Data = (char*)s_UStringAllocator->Allocate(sizeof(char), 8);
		strcpy(m_Data, "");
	}

	UString::UString(const char* str)
	{
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str) + 1, 8);
		strcpy(m_Data, str);
	}

	UString::UString(int size)
	{
		m_Data = (char*)s_UStringAllocator->Allocate(sizeof(char) * size, 8);
	}

	UString::UString(const UString& str)
	{
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str.m_Data) + 1, 8);
		strcpy(m_Data, str.m_Data);
	}

	UString::~UString()
	{
		s_UStringAllocator->Deallocate(m_Data);
	}

	const size_t UString::Size() const
	{
		return strlen(m_Data);
	}

	char* UString::RawString() const
	{
		return m_Data;
	}

	char* UString::Begin() const
	{
		return m_Data;
	}

	char* UString::End() const
	{
		return m_Data + strlen(m_Data);
	}

	UString UString::SubString(unsigned int start, unsigned int end)
	{
		size_t size = (size_t)end - start;
		if (size == 0)
			return "";
		ASSERT(end > start, "end cant be before start");
		ASSERT(end < Size(), "end index out of bounds");

		char* buffer = (char*)Memory::GetDefaultAlloc()->Allocate(size + 1, 8);

		for (int i = 0; i < end - start; i++)
		{
			buffer[i] = m_Data[start + i];
		}
		buffer[size] = '\0';

		Memory::GetDefaultAlloc()->Deallocate(buffer);

		return UString(buffer);
	}

	void UString::operator=(const UString& str)
	{
		s_UStringAllocator->Deallocate((void*)m_Data);
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str.m_Data) + 1, 8);
		strcpy(m_Data, str.m_Data);
	}
	
	void UString::operator=(const char* str)
	{
		s_UStringAllocator->Deallocate((void*)m_Data);
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str) + 1, 8);
		strcpy(m_Data, str);
	}
	
	bool UString::operator==(const UString& str)
	{
		if (Size() == strlen(str.m_Data))
		{
			for (int i = 0; i < Size(); i++)
			{
				if (m_Data[i] != str.m_Data[i])
					return false;
			}
			return true;
		}
		return false;
	}

	bool UString::operator==(const char* str)
	{
		return *m_Data == *str;
	}

	void UString::operator+=(const UString& str)
	{
		char* temp = m_Data;
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str.m_Data) + strlen(m_Data) + 1, 8);
		strcpy(m_Data, temp);
		strcpy(m_Data + strlen(m_Data), str.m_Data);
		s_UStringAllocator->Deallocate((void*)temp);
	}

	void UString::operator+=(const char* str)
	{
		char* temp = m_Data;
		m_Data = (char*)s_UStringAllocator->Allocate(strlen(str) + strlen(m_Data) + 1, 8);
		strcpy(m_Data, temp);
		strcpy(m_Data + strlen(m_Data), str);
		s_UStringAllocator->Deallocate((void*)temp);
	}

	UString UString::operator+(const UString& str)
	{
		char* temp = (char*)s_UStringAllocator->Allocate(strlen(m_Data) + strlen(str.m_Data) + 1, 8);
		strcpy(temp, m_Data);
		strcpy(temp + strlen(m_Data), str.m_Data);
		s_UStringAllocator->Deallocate(temp);
		return UString(temp);
	}

	UString UString::operator+(const char* str)
	{
		char* temp = (char*)s_UStringAllocator->Allocate(strlen(m_Data) + strlen(str) + 1, 8);
		strcpy(temp, m_Data);
		strcpy(temp + strlen(m_Data), str);
		s_UStringAllocator->Deallocate(temp);
		return UString(temp);
	}

	char& UString::operator[](size_t i)
	{
		return m_Data[i];
	}

	std::ostream& operator<<(std::ostream& os, const UString& str)
	{
		os << str.m_Data;
		return os;
	}

	std::ostream& operator<<(std::ostream& os, const UStringView& str)
	{
		for (int i = 0; i < str.m_Size; i++)
		{
			os << str[i];
		}
		return os;
	}

	UStringView::UStringView(UString& String, int start, int end) :
		m_Start(start), m_End(end), m_Size((size_t)end - start)
	{
		ASSERT(m_Size > 0, "can not have the end before the start");

		m_String = &String;
	}

	char& UStringView::operator[](size_t i) const
	{
		ASSERT(i <= m_Size, "index out of bounds");
		return m_String->m_Data[m_Start + i];
	}

}
