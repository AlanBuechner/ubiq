#include "pch.h"
#include "UString.h"
#include "Engine/Core/Memory/FreeListAllocator.h"

namespace Engine
{
	FreeListAllocator* UString::s_UStringAllocator = new FreeListAllocator(sizeof(char) * 1000000, FreeListAllocator::FindFirst, 8);

	UString::UString()
	{
		Resize(0);
	}

	UString::UString(const char* str)
	{
		Resize(strlen(str));
		CopyOver(str);
	}

	UString::UString(int num)
	{
		Resize(IntLength(num));
		_itoa(num, m_Data, 10);
	}

	UString::UString(const UString& str)
	{
		Resize(str.Size());
		CopyOver(str.RawString());
	}

	UString::~UString()
	{
		Deallocate(m_Data);
		//s_UStringAllocator->TakeSnapShot();
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
		ASSERT(end >= start, "end cant be before start");
		ASSERT(end <= Size(), "end index out of bounds");

		char* buffer = (char*)Memory::GetDefaultAlloc()->Allocate(size + 1, 8);

		for (unsigned int i = 0; i < end - start; i++)
		{
			buffer[i] = m_Data[start + i];
		}
		buffer[size] = '\0';

		Memory::GetDefaultAlloc()->Deallocate(buffer);

		return UString(buffer);
	}

	int UString::Find(const UString& match)
	{
		size_t matchSize = match.Size();
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
					return i;
				}
			}
		}
		return -1;
	}

	int UString::Find(const char match)
	{
		for (int i = 0; i < Size(); i++)
		{
			if (m_Data[i] == match)
				return i;
		}
		return -1;
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

	void UString::operator=(const int num)
	{
		Resize(IntLength(num));
		_itoa(num, m_Data, 10);
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

	void UString::operator+=(const int num)
	{
		size_t size = Size();
		Resize(size + IntLength(num));
		_itoa(num, m_Data+size, 10);
	}

	UString UString::operator+(const UString& str) const
	{
		char* temp = (char*)Allocate(strlen(m_Data) + strlen(str.m_Data) + 1);
		strcpy(temp, m_Data);
		strcat(temp, str.m_Data);
		Deallocate(temp);
		return UString(temp);
	}

	UString UString::operator+(const char* str) const
	{
		char* temp = (char*)Allocate(strlen(m_Data) + strlen(str) + 1);
		strcpy(temp, m_Data);
		strcat(temp, str);
		Deallocate(temp);
		return UString(temp);
	}

	UString UString::operator+(const int num) const
	{
		size_t numSize = IntLength(num);
		char* temp = (char*)Allocate(strlen(m_Data) + numSize + 1);
		strcpy(temp, m_Data);
		_itoa(num, temp+strlen(m_Data), 10);
		Deallocate(temp);
		return UString(temp);
	}

	char& UString::operator[](size_t i) const
	{
		return m_Data[i];
	}

	UString::operator char()
	{
		return m_Data[0];
	}

	void UString::Resize(size_t size)
	{
		m_Size = size;
		if (m_Data != nullptr)
		{
			if (!s_UStringAllocator->ResizeAllocation(m_Data, size))
			{
				// cant resize allocation
				char* temp = m_Data;
				m_Data = (char*)Allocate(size + 1);
				//s_UStringAllocator->TakeSnapShot();

				CopyOver(temp);
				Deallocate(temp);
				//s_UStringAllocator->TakeSnapShot();
			}
			else // resized allocation
			{
				//s_UStringAllocator->TakeSnapShot();
			}
		}
		else
		{
			m_Data = (char*)Allocate(size + 1);
			//s_UStringAllocator->TakeSnapShot();
		}
		m_Data[size] = 0; // null termenats the end of the string
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

	void* UString::Allocate(size_t size) const
	{
		return s_UStringAllocator->Allocate(size, 8);
	}

	void UString::Deallocate(void* p) const
	{
		s_UStringAllocator->Deallocate(p);
	}

	size_t UString::IntLength(int num)
	{
		if (num == 0)
		{
			return 1;
		}
		int numSize = 0;
		if (num < 0)
		{
			num = -num;
			numSize++;
		}
		while (num > 0)
		{
			num /= 10;
			numSize++;
		}
		return numSize;
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
