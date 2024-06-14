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
	f"{includeDirs['ImGuizmo']}/ImGuizmo.cpp"
]

resources = [
	f"embeded/**.rc",
]

headers = [
	f"src/**.h",
	f"{includeDirs['stb_image']}/**.h",
	f"{includeDirs['ImGuizmo']}/ImGuizmo.h",
]

includes = [
	projDir,
	f"src",
	f"vendor",
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
	f"imgui",
	f"yaml-cpp",
	f"Reflection",
	f"Utilities",
	f"ProjectManager",
]

links = [
	f"{libs['Assimp']}",
	f"{libs['dxc']}",
	f"{libs['pix']}",
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
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	proj.intDir = GetIntDir(projName)
	proj.binDir = GetBinDir(projName)
	proj.genReflection = True
	return proj
