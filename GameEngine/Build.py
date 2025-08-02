# engine build script

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
	f"{includeDirs['stb_image']}/**.cpp",
]

resources = [
	f"embeded/**.rc",
]

headers = [
	f"src/**.h",
	f"{includeDirs['stb_image']}/**.h",
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
	f"{includeDirs['Reflection']}",
	f"{includeDirs['ProjectManager']}",
	f"{includeDirs['Utilities']}",
	f"{includeDirs['Agility']}",
	f"{includeDirs['tracy']}",
]

sysIncludes = []
sysIncludes.extend(GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
]

dependancys = [
	f"imgui",
	f"yaml-cpp",
	f"Reflection",
	f"Utilities",
	f"ProjectManager",
]

links = [
	f"{libs['Assimp']}",
	f"{libs['pix']}",
]

dlls = [
	{
		"folder" : "D3D12",
		"files" : [
			f"{vendorDirs['Agility']}/build/native/bin/x64/D3D12Core.dll",
			f"{vendorDirs['Agility']}/build/native/bin/x64/d3d12SDKLayers.dll",
		]
	},
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
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	proj.intDir = GetIntDir(projName)
	proj.binDir = GetBinDir(projName)
	proj.genReflection = True
	return proj
