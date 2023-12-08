#include "pch.h"
#include "DirectX12GPUProfiler.h"
#include "DX.h"

#include "Engine/Core/Application.h"

#define USE_PIX
#include "WinPixEventRuntime/pix3.h"

bool Engine::DirectX12GPUProfiler::s_CaptureFrame = false;

namespace Engine
{

	static std::wstring GetLatestWinPixGpuCapturerPath()
	{
		LPWSTR programFilesPath = nullptr;
		SHGetKnownFolderPath(FOLDERID_ProgramFiles, KF_FLAG_DEFAULT, NULL, &programFilesPath);

		fs::path pixInstallationPath = fs::path(programFilesPath) / "Microsoft PIX";

		std::wstring newestVersionFound;
		if (fs::exists(pixInstallationPath))
		{
			for (auto const& directory_entry : fs::directory_iterator(pixInstallationPath))
			{
				if (directory_entry.is_directory())
				{
					if (newestVersionFound.empty() || newestVersionFound < directory_entry.path().filename().c_str())
						newestVersionFound = directory_entry.path().filename().c_str();
				}
			}
		}

		if (newestVersionFound.empty())
		{
			// TODO: Error, no PIX installation found
			CORE_WARN("Pix is not installed");
			return L"";
		}

		return pixInstallationPath / newestVersionFound / L"WinPixGpuCapturer.dll";
	}

	void DirectX12GPUProfiler::Init()
	{
		if (GetModuleHandle(L"WinPixGpuCapturer.dll") == 0)
		{
			std::wstring pixPath = GetLatestWinPixGpuCapturerPath();
			if (pixPath == L"")
				CORE_WARN("Could not locate Pix instolation");
			else
			{
				CORE_INFO("Loading Pix");
				if (LoadLibrary(pixPath.c_str()) == NULL)
					CORE_WARN("Could not load pix");
			}
		}
	}

	void DirectX12GPUProfiler::BeginEvent(Ref<DirectX12CommandList> commandList, const std::string& eventName)
	{
		PIXBeginEvent(commandList->GetCommandList(), 0, eventName.c_str());
	}

	void DirectX12GPUProfiler::EndEvent(Ref<DirectX12CommandList> commandList)
	{
		PIXEndEvent(commandList->GetCommandList());
	}

	void DirectX12GPUProfiler::SetTargetWindow(void* nativeWindow)
	{
		//PIXSetTargetWindow((HWND)nativeWindow);
		//PIXSetHUDOptions(PIX_HUD_SHOW_ON_TARGET_WINDOW_ONLY);
	}

	void DirectX12GPUProfiler::TriggerPixGPUCapture()
	{
		CORE_INFO("starting gpu capture");
		//CORE_ASSERT_HRESULT(PIXGpuCaptureNextFrames(L"LoadScene.wpix", 2), "Failed to Capture Frames");

		s_CaptureFrame = true;
	}

	void DirectX12GPUProfiler::BeginFrame()
	{
		if (s_CaptureFrame)
		{
			PIXCaptureParameters params;
			params.GpuCaptureParameters.FileName = L"Capture.wpix";
			PIXBeginCapture(PIX_CAPTURE_GPU, &params);
		}
	}

	void DirectX12GPUProfiler::EndFrame()
	{
		if (s_CaptureFrame)
		{
			PIXEndCapture(FALSE);
			s_CaptureFrame = false;
		}
	}

}
