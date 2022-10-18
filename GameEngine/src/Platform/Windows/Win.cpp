#include "pch.h"
#include "Win.h"

std::wstring TranslateError(HRESULT hr)
{
	wchar_t* pMsgBuf = nullptr;
	DWORD nMsgLen = FormatMessage(FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM | FORMAT_MESSAGE_IGNORE_INSERTS, 
		nullptr, hr, MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT),
		reinterpret_cast<LPWSTR>(&pMsgBuf), 0, nullptr);
	std::wstring errorString = pMsgBuf;
	if (nMsgLen == 0)
		errorString = L"Unidentified error code";
	LocalFree(pMsgBuf);
	return errorString;
}
