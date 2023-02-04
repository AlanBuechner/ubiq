powershell -ExecutionPolicy Bypass -File GetDXC.ps1 ..\GameEngine\vendor\dxc

(
	echo ^<Project^>
	echo  ^<PropertyGroup^>
	echo    ^<LLVMInstallDir^>%cd%\..\vendor\Compiler^</LLVMInstallDir^>
	echo    ^<LLVMToolsVersion^>16^</LLVMToolsVersion^>
	echo  ^</PropertyGroup^>
	echo ^</Project^>
) > "../Directory.build.props"

rem echo  \<PropertyGroup\>
rem echo    <LLVMInstallDir>%cd%\vendor\Compiler</LLVMInstallDir>
rem echo    <LLVMToolsVersion>16</LLVMToolsVersion>
rem echo  </PropertyGroup>
rem echo </Project>

GenerateProjects.bat