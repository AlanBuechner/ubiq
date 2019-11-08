#include "pch.h"
#include "UFileIO.h"
#ifdef PLATFORM_WINDOWS
#include <Windows.h>
#endif // PLATFORM_WINDOWS


namespace Engine
{
#ifdef PLATFORM_WINDOWS

	bool Engine::UFileIO::Open(UString& path)
	{
		HANDLE hFile = CreateFileA((LPCSTR)path.RawString(), 
			GENERIC_READ | GENERIC_WRITE, 
			FILE_SHARE_READ | FILE_SHARE_WRITE, 
			NULL, 
			CREATE_NEW, 
			FILE_ATTRIBUTE_ENCRYPTED | FILE_ATTRIBUTE_HIDDEN,
			NULL);

		if (hFile == INVALID_HANDLE_VALUE)
		{
			DEBUG_ERROR("Cant open File {0} Error code: {1}", path, GetLastError());
			return false;
		}
		m_hFile = hFile;
		return true;
	}

	Ref<UString> UFileIO::ReadFile(int charsToRead)
	{
		Ref<UString> str = CreateSharedPtr<UString>(charsToRead);

		char* buffer = str->RawString();

		return str;
		
	}

#endif // PLATFORM_WINDOWS
}