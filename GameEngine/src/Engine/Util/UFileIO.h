#pragma once
#include "Engine/Core/core.h"
#include "UString.h"

namespace Engine
{
#ifdef PLATFORM_WINDOWS
	class UFileIO
	{
	public:
		UFileIO();

		~UFileIO()
		{

		}

		bool Open(const UString& path);

		void Close();

		unsigned int GetFileSize() const;

		long GetFilePointer();

		void SetCursor(unsigned int pos);

		char* ReadFromFile(unsigned int charsToRead = 0, unsigned int offset = 0);

		void WriteToFile(UString& data);

	private:
		HANDLE m_hFile = nullptr;

		UString m_Path;

		PBYTE m_Str = nullptr;
	};
#endif // PLATFORM_WINDOWS

}