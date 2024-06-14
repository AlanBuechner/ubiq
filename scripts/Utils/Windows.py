import Config

import subprocess
import re
import os

def GetLatestDir(dir):
	dirs = []
	for d in os.listdir(dir):
		if(d[0].isdigit()):
			dirs.append(d)
	return dir + sorted(dirs)[-1]

def GetVisualStudioDirectory():
	return "C:/Program Files/Microsoft Visual Studio/2022/Community/"

def GetMSVCDirectory():
	return GetLatestDir(GetVisualStudioDirectory() + "VC/Tools/MSVC/")

def CashFMSVersion():
	args = ["cl.exe"]
	path = GetMSVCDirectory() + "/bin/Hostx64/x64"
	result = subprocess.run(args, cwd=path, shell=True, capture_output=True, text=True)
	return re.search(r"\d+\.\d+\.\d+", result.stderr).group() # the version number can be found in the error output and not the standerd output for some reason
fms_version = CashFMSVersion()

def GetFMSVersion():
	return fms_version

def GetWindowsKit():
	return "C:/Program Files (x86)/Windows Kits"

def GetWindowsKitInclude():
	return GetLatestDir(f"{GetWindowsKit()}/10/Include/")

def GetWindowsKitBin():
	return GetLatestDir(f"{GetWindowsKit()}/10/bin/")

def GetNETFXSDK():
	return GetLatestDir("C:/Program Files (x86)/Windows Kits/NETFXSDK/")

def GetWindowsIncludes():
	vsDir = GetVisualStudioDirectory()
	msvcDir = GetMSVCDirectory()
	windowsKitIncludeBase = GetWindowsKitInclude()
	NETFXSDK = GetNETFXSDK()

	includes = []
	includes.append(f"{msvcDir}/include")
	includes.append(f"{msvcDir}/atlmfc/include")
	includes.append(f"{vsDir}/VC/Auxiliary/VS/include")
	includes.append(f"{vsDir}/VC/Auxiliary/VS/UnitTest/include")
	includes.append(f"{windowsKitIncludeBase}/ucrt")
	includes.append(f"{windowsKitIncludeBase}/um")
	includes.append(f"{windowsKitIncludeBase}/shared")
	includes.append(f"{windowsKitIncludeBase}/winrt")
	includes.append(f"{windowsKitIncludeBase}/cppwinrt")
	includes.append(f"{NETFXSDK}4.8/Include/um")
	return includes
