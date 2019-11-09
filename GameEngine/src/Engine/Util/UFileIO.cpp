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
	}

	Ref<UString> UFileIO::ReadFromFile(int charsToRead)
	{
		if (charsToRead == 0)
		{
			struct stat stat_buf;
			int rc = stat(m_Path->RawString(), &stat_buf);
			charsToRead = stat_buf.st_size;
			if (rc != 0)
			{
				DEBUG_ERROR("cant Read file size of {0}", *m_Path);
				__debugbreak();
				return nullptr;
			}
		}

		char* buffer = (char*)Memory::GetDefaultAlloc()->Allocate((size_t)charsToRead, 8);

		DWORD bytesRead;

		if (!ReadFile(m_hFile, (void*)buffer, charsToRead, &bytesRead, NULL))
		{
			DEBUG_ERROR("Can not Read From file {0} Error code: {1}", *m_Path, GetLastError());
			__debugbreak();
			return nullptr;
		}

		Ref<UString> str = CreateSharedPtr<UString>(bytesRead);

		*str = buffer;

		Memory::GetDefaultAlloc()->Deallocate(buffer);

		return str;
	}

#endif // PLATFORM_WINDOWS
}