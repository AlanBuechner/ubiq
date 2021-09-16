#include "pch.h"
#include "Utils.h"

namespace Engine
{

	const wchar_t* GetWStr(const char* str)
	{
		const size_t size = strlen(str) + 1;
		wchar_t* wstr = new wchar_t[size];
		mbstowcs(wstr, str, size);
		return wstr;
	}

	const char* GetStr(const wchar_t* wstr)
	{
		const size_t size = wcslen(wstr) + 1;
		char* str = new char[size];
		wcstombs(str, wstr, size);
		return str;
	}

}