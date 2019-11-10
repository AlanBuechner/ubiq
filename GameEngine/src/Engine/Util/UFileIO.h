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

		bool Open(const UString& path);

		void Close();

		unsigned int GetFileSize() const;

		void SetCursor(int pos);

		char* ReadFromFile(unsigned int charsToRead = 0, unsigned int start = 0);

	private:
		HANDLE m_hFile = nullptr;

		const UString* m_Path;

		PBYTE m_Str = nullptr;
	};
#endif // PLATFORM_WINDOWS

}