# USG-Editor build script

import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None)).replace("\\", "/")
projName = os.path.basename(projDir)

pchHeader = f"{location}/src/pch.h"
pchSource = f"{location}/src/pch.cpp"

sources = [
	f"src/**.cpp",
]

resources = [
	f"embeded/**.rc",
]

headers = [
	f"src/**.h",
]

includes = [
	projDir,
	f"src",
	f"{includeDirs['vendor']}",
	f"{includeDirs['ImGui']}",
	f"{includeDirs['glm']}",
	f"{includeDirs['stb_image']}",
	f"{includeDirs['yaml']}",
	f"{includeDirs['ImGuizmo']}",
	f"{includeDirs['Assimp']}",
	f"{includeDirs['json']}",
	f"{includeDirs['dxc']}",
	f"{includeDirs['pix']}",
	f"{includeDirs['Aftermath']}",
	f"{includeDirs['ProjectManager']}/src",
	f"{includeDirs['Reflection']}/src",
	f"{includeDirs['Utilities']}/src",
	f"{location}/GameEngine/vendor",
	f"{location}/GameEngine/src",
	f"{location}/GameEngine/embeded",
]

sysIncludes = [

]
sysIncludes.extend(GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
]

dependancys = [
	"GameEngine",
	"ProjectManager",
]
if(Config.gameProject != None):
	dependancys.append(os.path.basename(Config.gameProject))

links = [
	"kernel32.lib",
	"user32.lib",
	"comdlg32.lib",
	"shell32.lib",
]

dlls = [
	f"{vendorDirs['dxc']}/bin/x64/dxil.dll",
	f"{vendorDirs['dxc']}/bin/x64/dxcompiler.dll",
	f"{vendorDirs['Assimp']}/assimp-vc140-mt.dll",
	f"{vendorDirs['pix']}/bin/x64/WinPixEventRuntime.dll",
	f"{projDir}/zlibd.dll",
]

def GetProject():

	proj = BuildUtils.ProjectEnviernment()
	proj.projectDirectory = projDir
	proj.pchSource = pchSource
	proj.pchHeader = pchHeader
	proj.sources = sources
	proj.resources = resources
	proj.headers = headers
	proj.includes = includes
	proj.sysIncludes = sysIncludes
	proj.defines = defines
	proj.links = links
	proj.dlls = dlls
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.EXECUTABLE
	proj.genReflection = True
	proj.intDir = GetIntDir(projName, Config.gameProject)
	proj.binDir = GetBinDir(projName, Config.gameProject)
	return proj
