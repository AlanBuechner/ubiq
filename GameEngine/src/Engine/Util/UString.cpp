#include "pch.h"
#include "UString.h"
#include "Engine/Core/Memory/FreeListAllocator.h"

namespace Engine
{
	FreeListAllocator* UString::alloc = new FreeListAllocator(sizeof(char) * 1000, FreeListAllocator::FindFirst);

	UString::UString()
	{
	}

	UString::UString(const char* str)
	{
		m_Data = (char*)alloc->Allocate(strlen(str), 8);
		strcpy(m_Data, str);
	}

	UString::UString(int size)
	{
		m_Data = (char*)alloc->Allocate(sizeof(char) * size, 8);
	}

	UString::UString(const UString& str)
	{
		if(m_Data != nullptr)
			alloc->Deallocate((void*)m_Data);
		m_Data = (char*)alloc->Allocate(strlen(str.m_Data), 8);
		strcpy(m_Data, str.m_Data);
	}

	int UString::Size()
	{
		return strlen(m_Data);
	}

	void UString::operator=(const UString& str)
	{
		if (m_Data != nullptr)
			alloc->Deallocate((void*)m_Data);
		m_Data = (char*)alloc->Allocate(strlen(str.m_Data), 8);
		strcpy(m_Data, str.m_Data);
	}

	bool UString::operator==(const UString& str)
	{
		return *m_Data == *str.m_Data;
	}

	void UString::operator+=(const UString& str)
	{
		char* temp = m_Data;
		m_Data = (char*)alloc->Allocate(strlen(str.m_Data) + strlen(m_Data), 8);
		strcpy(m_Data, temp);
		strcpy(m_Data + strlen(m_Data), str.m_Data);
		alloc->Deallocate((void*)m_Data);
	}

	UString UString::operator+(const UString& str)
	{
		char* temp = (char*)alloc->Allocate(strlen(m_Data) + strlen(str.m_Data) ,8);
		strcpy(temp, m_Data);
		strcpy(temp + strlen(m_Data), str.m_Data);
		alloc->Deallocate(temp);
		return UString(temp);
	}

	std::ostream& operator<<(std::ostream& os, const UString& str)
	{
		os << str.m_Data;
		return os;
	}

}
