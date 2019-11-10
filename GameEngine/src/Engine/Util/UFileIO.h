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

		char* ReadFromFile(int charsToRead = 0);

	private:
		HANDLE m_hFile = nullptr;

		const UString* m_Path;

		PBYTE m_Str;
	};
#endif // PLATFORM_WINDOWS

}