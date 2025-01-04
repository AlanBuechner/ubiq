# test project build script

import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None)).replace("\\", "/")
projName = os.path.basename(projDir)

inEditor = True #Config.project == "USG-Editor"

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
	f"{includeDirs['ProjectManager']}",
	f"{includeDirs['Reflection']}",
	f"{includeDirs['Utilities']}",
	f"{location}/GameEngine/src",
	f"{location}/GameEngine/embeded",
	f"{projDir}/modules",
]

sysIncludes = []
sysIncludes.extend(GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
]

vsDefines = [
	"EDITOR"
]

dependancys = [
	"TestModule",
]

links = []

modules = [
	"modules/TestModule",
	f"{location}/EditorModule",
]

if(inEditor):
	defines.append("EDITOR")
	includes.append(f"{location}/EditorModule/src")
	dependancys.append("EditorModule")

def GetProject():
	proj = BuildUtils.ProjectEnviernment()
	proj.projectDirectory = projDir
	proj.pchSource = ""
	proj.pchHeader = ""
	proj.sources = sources
	proj.resources = resources
	proj.headers = headers
	proj.includes = includes
	proj.sysIncludes = sysIncludes
	proj.defines = defines
	proj.vsDefines = vsDefines
	proj.links = links
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	buildLoc = "/Runtime"
	if(inEditor):
		buildLoc = "/Editor"
	proj.intDir = GetIntDir(projName, Config.gameProject) + buildLoc
	proj.binDir = GetBinDir(projName, Config.gameProject) + buildLoc
	proj.genReflection = True
	return proj


def GetModules():
	return modules
