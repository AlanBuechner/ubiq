#include "pch.h"
#include "UString.h"
#include "Engine/Core/Memory/FreeListAllocator.h"

namespace Engine
{
	FreeListAllocator* UString::s_UStringAllocator = new FreeListAllocator(sizeof(char) * 10000, FreeListAllocator::FindFirst);

	UString::UString()
	{
		Resize(0);
	}

	UString::UString(const char* str)
	{
		Resize(strlen(str));
		CopyOver(str);
	}

	UString::UString(int size)
	{
		Resize(size);
	}

	UString::UString(const UString& str)
	{
		Resize(str.Size());
		CopyOver(str.RawString());
	}

	UString::~UString()
	{
		s_UStringAllocator->Deallocate(m_Data);
	}

	const size_t UString::Size() const
	{
		return m_Size;
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

		for (unsigned int i = 0; i < end - start; i++)
		{
			buffer[i] = m_Data[start + i];
		}
		buffer[size] = '\0';

		Memory::GetDefaultAlloc()->Deallocate(buffer);

		return UString(buffer);
	}

	bool UString::Find(const UString& match)
	{
		size_t matchSize = strlen(match.RawString());
		if (matchSize > Size())
		{
			return false;
		}

		for (int i = 0; i < Size() - matchSize; i++)
		{
			for (int j = 0; j < matchSize; j++)
			{
				if (m_Data[i + j] != match[j])
					break;
				else if (j == matchSize - 1)
				{
					return true;
				}
			}
		}
		return false;
	}

	void UString::operator=(const UString& str)
	{
		Resize(str.Size());
		CopyOver(str.RawString());
	}
	
	void UString::operator=(const char* str)
	{
		Resize(strlen(str));
		CopyOver(str);
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
		if (Size() == strlen(str))
		{
			for (int i = 0; i < Size(); i++)
			{
				if (m_Data[i] != str[i])
					return false;
			}
			return true;
		}
		return false;
	}

	void UString::operator+=(const UString& str)
	{
		Resize(strlen(str.m_Data) + strlen(m_Data) + 1);
		strcat(m_Data, str.m_Data);
	}

	void UString::operator+=(const char* str)
	{
		Resize(strlen(str) + strlen(m_Data) + 1);
		strcat(m_Data, str);
	}

	UString UString::operator+(const UString& str)
	{
		char* temp = (char*)s_UStringAllocator->Allocate(strlen(m_Data) + strlen(str.m_Data) + 1, 8);
		strcpy(temp, m_Data);
		strcat(temp, str.m_Data);
		s_UStringAllocator->Deallocate(temp);
		return UString(temp);
	}

	UString UString::operator+(const char* str)
	{
		char* temp = (char*)s_UStringAllocator->Allocate(strlen(m_Data) + strlen(str) + 1, 8);
		strcpy(temp, m_Data);
		strcat(temp, str);
		s_UStringAllocator->Deallocate(temp);
		return UString(temp);
	}

	char& UString::operator[](size_t i) const
	{
		return m_Data[i];
	}

	void UString::Resize(size_t size)
	{
		m_Size = size;
		char* temp = m_Data;
		m_Data = (char*)s_UStringAllocator->Allocate(size + 1, 8);
		s_UStringAllocator->TakeSnapShot();
		m_Data[size] = 0;
		if (temp != nullptr)
		{
			CopyOver(temp);
			s_UStringAllocator->Deallocate(temp);
			s_UStringAllocator->TakeSnapShot();
		}
	}

	void UString::CopyOver(const char* data)
	{
		if (data != nullptr && m_Data != nullptr)
		{
			for (int i = 0; i < m_Size; i++)
			{
				m_Data[i] = data[i];
				if (m_Data[i] == '\0')
					break;
			}
		}
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
