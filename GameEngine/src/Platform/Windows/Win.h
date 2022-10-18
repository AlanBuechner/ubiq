#pragma once
#include "Engine/Core/Core.h"

#if defined(PLATFORM_WINDOWS)

// target Windows 10 or later
#define _WIN32_WINNT 0x0A00

#include <sdkddkver.h>

#ifndef FULL_WINTARD
#define WIN32_LEAN_AND_MEAN
#define NOGDICAPMASKS
#define NOSYSMETRICS
#define NOMENUS
#define NOICONS
#define NOSYSCOMMANDS
#define NORASTEROPS
#define OEMRESOURCE
#define NOATOM
#define NOCLIPBOARD
#define NOCOLOR
#define NOCTLMGR
#define NODRAWTEXT
#define NOKERNEL
#define NONLS
#define NOMEMMGR
#define NOMETAFILE
#define NOOPENFILE
#define NOSCROLL
#define NOSERVICE
#define NOSOUND
#define NOTEXTMETRIC
#define NOWH
#define NOCOMM
#define NOKANJI
#define NOHELP
#define NOPROFILER
#define NODEFERWINDOWPOS
#define NOMCX
#define NORPC
#define NOPROXYSTUB
#define NOIMAGE
#define NOTAPE
#endif

#define NOMINMAX

#include <Windows.h>
#include <comdef.h>
#include <string>

std::wstring TranslateError(HRESULT hr);

#define CORE_ASSERT_HANDEL(x, ...) CORE_ASSERT((x) != INVALID_HANDLE_VALUE, __VA_ARGS__);
#define CORE_ASSERT_HRESULT(x, msg) {HRESULT hr__LINE__ = (x); if(FAILED(hr__LINE__)) { \
std::wstring err__LINE__ = TranslateError(hr__LINE__);\
CORE_ERROR("HRESULT Assertion Failed: {0} \n{1}", msg, ::Engine::GetStr(err__LINE__)); __debugbreak();}}

#endif
