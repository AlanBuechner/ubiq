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

	bool Engine::UFileIO::Open(const UString& path, bool trunk)
	{
		if (m_hFile != 0)
			Close();

		HANDLE hFile = CreateFileA((LPCSTR)path.RawString(), 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			CREATE_NEW | OPEN_EXISTING | (trunk & TRUNCATE_EXISTING),
			FILE_ATTRIBUTE_NORMAL,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE && GetLastError() == 2)
		{
			hFile = CreateFileA((LPCSTR)path.RawString(),
				GENERIC_READ | GENERIC_WRITE,
				FILE_SHARE_READ | FILE_SHARE_WRITE,
				NULL,
				CREATE_ALWAYS | (trunk & TRUNCATE_EXISTING),
				FILE_ATTRIBUTE_NORMAL,
				NULL);
		}

		if (hFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_ERROR("Cant open File {0} Error code: {1}", path, GetLastError());
			__debugbreak();
			Close();
			return false;
		}

		if (trunk)
		{
			SetEndOfFile(hFile);
		}

		m_hFile = hFile;
		m_Path = path;
		m_IsOpen = true;
		return true;
	}

	void UFileIO::Close()
	{
		CloseHandle(m_hFile);
		if(m_Str != nullptr)
			Memory::GetDefaultAlloc()->Deallocate(m_Str);
		m_Str = nullptr;
		m_hFile = nullptr;
		m_IsOpen = false;
	}

	unsigned int UFileIO::GetFileSize() const
	{
		LARGE_INTEGER fileSize;
		if (!GetFileSizeEx(m_hFile, &fileSize))
		{
			DEBUG_ERROR("Cant Read file size of {0}", m_Path);
			__debugbreak();
		}
		return (unsigned int)fileSize.QuadPart;
	}

	long UFileIO::GetFilePointer()
	{
		return SetFilePointer(m_hFile, 0, NULL, FILE_CURRENT);
	}

	void UFileIO::SetCursor(unsigned int pos)
	{
		if (SetFilePointer(m_hFile, pos, NULL, FILE_BEGIN) == INVALID_SET_FILE_POINTER)
		{
			DEBUG_ERROR("Cant set file Pointer Error code: {0}", GetLastError());
			__debugbreak();
		}
	}

	char* UFileIO::ReadFromFile(unsigned int charsToRead, unsigned int offset)
	{
		// if charsToRead is 0 read the rest of the file
		if (charsToRead == 0)
		{
			charsToRead = GetFileSize(); // gets the file size
			if (charsToRead == NULL)
			{
				return nullptr;
			}
			charsToRead -= offset;
		}

		// sets the file pointer
		if (offset > 0)
		{
			if (SetFilePointer(m_hFile, offset, NULL, FILE_CURRENT) == INVALID_SET_FILE_POINTER)
			{
				DEBUG_ERROR("Cant set file Pointer Error code: {0}", GetLastError());
				__debugbreak();
				Close();
			}
		}

		PBYTE buffer = (PBYTE)Memory::GetDefaultAlloc()->Allocate((size_t)charsToRead * sizeof(char), 8); // alocate memory for the buffer

		DWORD bytesRead;

		if (!ReadFile(m_hFile, (void*)buffer, charsToRead * sizeof(char), &bytesRead, NULL)) // read from the file and save it to the buffer
		{
			DEBUG_ERROR("Cant Read From file {0} Error code: {1}", m_Path, GetLastError());
			__debugbreak();
			Close();
			return nullptr;
		}

		buffer[bytesRead] = '\0'; // null terminat the end of the file

		// dealocats the memory from the last read to prevent memory leak
		if(m_Str != nullptr)
			Memory::GetDefaultAlloc()->Deallocate(m_Str);

		// allocate memory for the new buffer 
		// nesesary if the chars the user wanted to read was larger than the size of the file
		m_Str = (PBYTE)Memory::GetDefaultAlloc()->Allocate(strlen((char*)buffer) * sizeof(char), 8); 

		strcpy((char*)m_Str, (char*)buffer); // copy over the data

		Memory::GetDefaultAlloc()->Deallocate(buffer); // dealocate the memory used by the buffer

		return (char*)m_Str;
	}

	void UFileIO::WriteToFile(UString& data)
	{
		DWORD dwBytesWritten = 0;
		int errorFlags = WriteFile(m_hFile, data.RawString(), (DWORD)data.Size(), &dwBytesWritten, nullptr);

		if (errorFlags == FALSE)
		{
			DEBUG_ERROR("Cant Write to file {0} Error code: {1}", m_Path, GetLastError());
			Close();
		}
	}

#endif // PLATFORM_WINDOWS
}