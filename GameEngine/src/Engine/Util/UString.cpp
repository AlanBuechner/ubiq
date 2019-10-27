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

	int UString::Size()
	{
		return strlen(m_Data);
	}

	char* UString::RawString()
	{
		return m_Data;
	}

	char* UString::Begin()
	{
		return m_Data;
	}

	char* UString::End()
	{
		return m_Data + strlen(m_Data);
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
		return *m_Data == *str.m_Data;
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

}
