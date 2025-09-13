# test project build script

import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None)).replace("\\", "/")
projName = os.path.basename(projDir)

inEditor = False

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
	f"{includeDirs['tracy']}",
	f"{location}/GameEngine/src",
	f"{location}/GameEngine/embeded",
	f"{projDir}/modules",
	f"{location}/modules",
	f"{location}/modules/PostProcessingModule/src",
	f"{location}/modules/RenderingUtilsModule/src",
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
	"PostProcessingModule",
	"RenderingUtilsModule",
	"GameEngine",
]

links = [
	"kernel32.lib",
	"user32.lib",
	"comdlg32.lib",
	"shell32.lib",
]

modules = [
	"modules/TestModule",
	f"{location}/modules/EditorModule",
	f"{location}/modules/PostProcessingModule",
	f"{location}/modules/RenderingUtilsModule",
]

if(inEditor):
	defines.append("EDITOR")
	includes.append(f"{location}/modules/EditorModule/src")
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
	proj.buildType = BuildUtils.BuildType.EXECUTABLE
	buildLoc = "/Runtime"
	if(inEditor):
		buildLoc = "/Editor"
	proj.intDir = GetIntDir(projName, projDir) + buildLoc
	proj.binDir = GetBinDir(projName, projDir) + buildLoc
	proj.genReflection = True
	return proj


def GetModules():
	return modules
