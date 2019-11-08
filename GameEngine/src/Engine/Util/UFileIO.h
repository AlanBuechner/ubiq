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

		bool Open(UString& path);

		Ref<UString> ReadFile(int charsToRead);

	private:
		HANDLE m_hFile = nullptr;
	};
#endif // PLATFORM_WINDOWS

}