@echo off

if not exist ..\vendor\python\ (
	echo "Installing python"
	mkdir ..\vendor\python
	powershell -command "Invoke-WebRequest -UseBasicParsing -Uri 'https://www.python.org/ftp/python/3.11.0/python-3.11.0-embed-amd64.zip' -OutFile '../vendor/python/python.zip'"
	powershell -command "Expand-Archive ..\vendor\python\python.zip ..\vendor\python"
	del ../vendor/python/python.zip
)

"../vendor/python/python.exe" PythonSetup.py
"../vendor/python/python.exe" Setup.py

:: download and install reqiered components
:: powershell -ExecutionPolicy Bypass -File GetDXC.ps1 ..\GameEngine\vendor\dxc
:: powershell -ExecutionPolicy Bypass -File GetPix.ps1 ..\GameEngine\vendor\pix
:: powershell -ExecutionPolicy Bypass -File GetCompiler.ps1 ..\vendor\Compiler
:: powershell -ExecutionPolicy Bypass -File GetVulkan.ps1

:: copy dlls
:: xcopy "..\GameEngine\vendor\dxc\bin\x64\dxil.dll" "..\USG-Editor\dxil.dll" /y
:: xcopy "..\GameEngine\vendor\dxc\bin\x64\dxil.dll" "..\Sandbox\dxil.dll" /y
:: 
:: xcopy "..\GameEngine\vendor\dxc\bin\x64\dxcompiler.dll" "..\USG-Editor\dxcompiler.dll" /y
:: xcopy "..\GameEngine\vendor\dxc\bin\x64\dxcompiler.dll" "..\Sandbox\dxcompiler.dll" /y
:: 
:: xcopy "..\GameEngine\vendor\Aftermath\lib\x64\GFSDK_Aftermath_Lib.x64.dll" "..\USG-Editor\GFSDK_Aftermath_Lib.x64.dll" /y
:: xcopy "..\GameEngine\vendor\Aftermath\lib\x64\GFSDK_Aftermath_Lib.x64.dll" "..\Sandbox\GFSDK_Aftermath_Lib.x64.dll" /y
:: 
:: xcopy "..\GameEngine\vendor\pix\bin\x64\WinPixEventRuntime.dll" "..\USG-Editor\WinPixEventRuntime.dll" /y
:: xcopy "..\GameEngine\vendor\pix\bin\x64\WinPixEventRuntime.dll" "..\Sandbox\WinPixEventRuntime.dll" /y

:: generate files
:: (
:: 	echo ^<Project^>
:: 	echo  ^<PropertyGroup^>
:: 	echo    ^<LLVMInstallDir^>%cd%\..\vendor\Compiler^</LLVMInstallDir^>
:: 	echo    ^<LLVMToolsVersion^>16^</LLVMToolsVersion^>
:: 	echo  ^</PropertyGroup^>
:: 	echo ^</Project^>
:: ) > "../Directory.build.props"

:: generate projects
:: GenerateProjects.bat
