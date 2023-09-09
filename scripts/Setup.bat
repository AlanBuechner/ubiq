@echo off
:: download and install reqiered components
powershell -ExecutionPolicy Bypass -File GetDXC.ps1 ..\GameEngine\vendor\dxc
powershell -ExecutionPolicy Bypass -File GetPix.ps1 ..\GameEngine\vendor\pix
powershell -ExecutionPolicy Bypass -File GetCompiler.ps1 ..\vendor\Compiler
powershell -ExecutionPolicy Bypass -File GetVulkan.ps1

:: copy dlls
xcopy "..\GameEngine\vendor\dxc\bin\x64\dxil.dll" "..\USG-Editor\dxil.dll" /y
xcopy "..\GameEngine\vendor\dxc\bin\x64\dxil.dll" "..\Sandbox\dxil.dll" /y

xcopy "..\GameEngine\vendor\dxc\bin\x64\dxcompiler.dll" "..\USG-Editor\dxcompiler.dll" /y
xcopy "..\GameEngine\vendor\dxc\bin\x64\dxcompiler.dll" "..\Sandbox\dxcompiler.dll" /y

xcopy "..\GameEngine\vendor\Aftermath\lib\x64\GFSDK_Aftermath_Lib.x64.dll" "..\USG-Editor\GFSDK_Aftermath_Lib.x64.dll" /y
xcopy "..\GameEngine\vendor\Aftermath\lib\x64\GFSDK_Aftermath_Lib.x64.dll" "..\Sandbox\GFSDK_Aftermath_Lib.x64.dll" /y

xcopy "..\GameEngine\vendor\pix\bin\x64\WinPixEventRuntime.dll" "..\USG-Editor\WinPixEventRuntime.dll" /y
xcopy "..\GameEngine\vendor\pix\bin\x64\WinPixEventRuntime.dll" "..\Sandbox\WinPixEventRuntime.dll" /y

:: generate files
(
	echo ^<Project^>
	echo  ^<PropertyGroup^>
	echo    ^<LLVMInstallDir^>%cd%\..\vendor\Compiler^</LLVMInstallDir^>
	echo    ^<LLVMToolsVersion^>16^</LLVMToolsVersion^>
	echo  ^</PropertyGroup^>
	echo ^</Project^>
) > "../Directory.build.props"

:: generate projects
GenerateProjects.bat