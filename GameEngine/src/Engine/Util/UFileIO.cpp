#include "pch.h"
#include "UFileIO.h"
#include "Engine/Core/Memory/Memory.h"
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#include <fileapi.h>
#endif // PLATFORM_WINDOWS


namespace Engine
{
#ifdef PLATFORM_WINDOWS

	UFileIO::UFileIO()
	{
	}

	bool Engine::UFileIO::Open(const UString& path)
	{
		HANDLE hFile = CreateFileA((LPCSTR)path.RawString(), 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			CREATE_NEW | OPEN_EXISTING,
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_ERROR("Cant open File {0} Error code: {1}", path, GetLastError());
			__debugbreak();
			return false;
		}
		m_hFile = hFile;
		m_Path = &path;
		return true;
	}

	void UFileIO::Close()
	{
		CloseHandle(m_hFile);
		Memory::GetDefaultAlloc()->Deallocate(m_Str);
	}

	char* UFileIO::ReadFromFile(int charsToRead)
	{
		// sets the chars to read to the size of the file
		if (charsToRead == 0) 
		{
			if (!GetFileSizeEx(m_hFile, (PLARGE_INTEGER)&charsToRead))
			{
				DEBUG_ERROR("cant Read file size of {0}", *m_Path);
				__debugbreak();
				return nullptr;
			}
		}

		PBYTE buffer = (PBYTE)Memory::GetDefaultAlloc()->Allocate((size_t)charsToRead * sizeof(char), 8); // alocate memory for the buffer

		DWORD bytesRead;

		if (!ReadFile(m_hFile, (void*)buffer, charsToRead * sizeof(char), &bytesRead, NULL)) // read from the file add save it to the buffer
		{
			DEBUG_ERROR("Can not Read From file {0} Error code: {1}", *m_Path, GetLastError());
			__debugbreak();
			return nullptr;
		}

		buffer[bytesRead] = '\0'; // null terminat the end of the file

		// allocate memory for the new buffer 
		// nesesary if the chars the user wanted to read was larger than the size of the file
		m_Str = (PBYTE)Memory::GetDefaultAlloc()->Allocate((size_t)bytesRead, 8); 

		strcpy((char*)m_Str, (char*)buffer); // copy over the data

		Memory::GetDefaultAlloc()->Deallocate(buffer); // dealocate the memory used by the buffer

		return (char*)m_Str;
	}

#endif // PLATFORM_WINDOWS
}