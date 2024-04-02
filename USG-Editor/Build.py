# USG-Editor build script

import scripts.BuildUtils as BuildUtils
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None))
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
	f"src",
	f"{includeDirs['vendor']}",
	f"{includeDirs['ImGui']}",
	f"{includeDirs['glm']}",
	f"{includeDirs['stb_image']}",
	f"{includeDirs['yaml']}",
	f"{includeDirs['ImGuizmo']}",
	f"{includeDirs['Box2D']}",
	f"{includeDirs['Assimp']}",
	f"{includeDirs['json']}",
	f"{includeDirs['dxc']}",
	f"{includeDirs['pix']}",
	f"{includeDirs['Aftermath']}",
	f"{includeDirs['ProjectManager']}/src",
	f"{location}/GameEngine/vendor",
	f"{location}/GameEngine/src",
	f"{location}/GameEngine/embeded",
]

sysIncludes = [
	f"{sysIncludeDirs['Compiler']}",
]
sysIncludes.extend(BuildUtils.GetSysIncludes())

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

links = [
	"kernel32.lib",
	"user32.lib",
	"comdlg32.lib",
	"shell32.lib",
]
links.extend(dependancys)

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
	proj.buildType = BuildUtils.BuildType.EXECUTABLE
	return proj
