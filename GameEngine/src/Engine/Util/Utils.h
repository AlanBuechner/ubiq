#pragma once
#include <wchar.h>
#include <cstring>
#include <stdlib.h>

namespace Engine
{
	const wchar_t* GetWStr(const char* str);
	const char* GetStr(const wchar_t* wstr);
}