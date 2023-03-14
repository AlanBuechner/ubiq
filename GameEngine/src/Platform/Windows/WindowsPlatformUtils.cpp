#include "pch.h"

#include "Engine/Util/PlatformUtils.h"
#include "Engine/Util/Utils.h"
#include "Engine/Core/Application.h"

#include <commdlg.h>

#include <shlwapi.h>
#include <shellapi.h>

namespace Engine
{
	HMODULE GetModule()
	{
		HMODULE hm = nullptr;
		GetModuleHandleEx(GET_MODULE_HANDLE_EX_FLAG_FROM_ADDRESS,
			(LPCTSTR)GetModule,
			&hm
		);
		return hm;
	}


	std::string FileDialogs::OpenFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetOpenFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return std::string();

	}

	std::string FileDialogs::SaveFile(const char* filter)
	{
		OPENFILENAMEA ofn;
		CHAR szFile[260] = { 0 };

		ZeroMemory(&ofn, sizeof(OPENFILENAME));
		ofn.lStructSize = sizeof(OPENFILENAME);
		ofn.hwndOwner = (HWND)Application::Get().GetWindow().GetNativeWindow();
		ofn.lpstrFile = szFile;
		ofn.nMaxFile = sizeof(szFile);
		ofn.lpstrFilter = filter;
		ofn.nFilterIndex = 1;
		ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST | OFN_NOCHANGEDIR;
		if (GetSaveFileNameA(&ofn) == TRUE)
		{
			return ofn.lpstrFile;
		}

		return std::string();
	}

	std::vector<std::string> GetCommandLineArguments()
	{
		LPWSTR cmd = GetCommandLineW();

		int numArgs;
		LPWSTR* args = CommandLineToArgvW(cmd, &numArgs);

		std::vector<std::string> argList;
		for (uint32 i = 0; i < numArgs; i++)
		{
			std::wstring arg(args[i]);
			argList.push_back(Engine::GetStr(arg));
		}

		LocalFree(args);

		return argList;
	}


	bool GetEmbededResource(uint32 type, uint32 id, byte*& data, uint32& size)
	{
		HMODULE module = GetModule();

		HRSRC res = FindResource(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		if (res == nullptr) return false;

		HGLOBAL hdata = LoadResource(module, res);
		if (hdata == nullptr) return false;

		size = SizeofResource(module, res);
		data = (byte*)LockResource(hdata);
		return true;
	}

	void UnloadEmbededResource(uint32 type, uint32 id)
	{
		HMODULE module = GetModule();

		HRSRC res = FindResource(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		if (res == nullptr) return;

		// TODO
	}


}
