#include "pch.h"
#include "Win.h"

#include "Engine/Util/PlatformUtils.h"
#include "Engine/Util/Utils.h"
#include "Engine/Core/Application.h"

#include <commdlg.h>

#include <shlwapi.h>
#include <shellapi.h>
#include <shlobj.h>

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
		CHAR szFile[MAX_PATH] = { 0 };

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
		CHAR szFile[MAX_PATH] = { 0 };

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


	static int CALLBACK BrowseCallbackProc(HWND hwnd, UINT uMsg, LPARAM lParam, LPARAM lpData)
	{

		if (uMsg == BFFM_INITIALIZED)
		{
			std::string tmp = (const char*)lpData;
			std::cout << "path: " << tmp << std::endl;
			SendMessage(hwnd, BFFM_SETSELECTION, TRUE, lpData);
		}

		return 0;
	}

	std::string FileDialogs::OpenFolder(std::string savedPath)
	{
		CHAR szFile[MAX_PATH] = { 0 };

		std::wstring wsavedPath(savedPath.begin(), savedPath.end());
		const wchar_t* pathParam = wsavedPath.c_str();

		BROWSEINFO bi = { 0 };
		bi.lpszTitle = L"Browse for folder...";
		bi.ulFlags = BIF_RETURNONLYFSDIRS | BIF_NEWDIALOGSTYLE;
		bi.lpfn = BrowseCallbackProc;
		bi.lParam = (LPARAM)pathParam;

		LPITEMIDLIST pidl = SHBrowseForFolder(&bi);

		if (pidl != 0)
		{
			//get the name of the folder and put it in path
			SHGetPathFromIDListA(pidl, szFile);

			//free memory used
			IMalloc* imalloc = 0;
			if (SUCCEEDED(SHGetMalloc(&imalloc)))
			{
				imalloc->Free(pidl);
				imalloc->Release();
			}

			return szFile;
		}

		return "";
	}


	Utils::Vector<std::string> GetCommandLineArguments()
	{
		LPWSTR cmd = GetCommandLineW();

		int numArgs;
		LPWSTR* args = CommandLineToArgvW(cmd, &numArgs);

		Utils::Vector<std::string> argList;
		for (uint32 i = 0; i < numArgs; i++)
		{
			std::wstring arg(args[i]);
			argList.Push(Engine::GetStr(arg));
		}

		LocalFree(args);

		return argList;
	}


	bool GetEmbededResource(uint32 type, uint32 id, byte*& data, uint32& size)
	{
		HMODULE module = GetModule();

		HRSRC res = FindResource(module, MAKEINTRESOURCE(id), MAKEINTRESOURCE(type));
		if (res == nullptr)
		{
			CORE_ASSERT_HRESULT(HRESULT_FROM_WIN32(GetLastError()), "Error failed to find resource");
			return false;
		}

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
