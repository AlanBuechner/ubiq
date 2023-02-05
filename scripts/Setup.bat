powershell -ExecutionPolicy Bypass -File GetDXC.ps1 ..\GameEngine\vendor\dxc
powershell -ExecutionPolicy Bypass -File GetCompiler.ps1 ..\vendor\Compiler

(
	echo ^<Project^>
	echo  ^<PropertyGroup^>
	echo    ^<LLVMInstallDir^>%cd%\..\vendor\Compiler^</LLVMInstallDir^>
	echo    ^<LLVMToolsVersion^>16^</LLVMToolsVersion^>
	echo  ^</PropertyGroup^>
	echo ^</Project^>
) > "../Directory.build.props"

GenerateProjects.bat