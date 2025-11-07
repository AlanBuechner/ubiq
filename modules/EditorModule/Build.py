# test project build script

import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None)).replace("\\", "/")
projName = os.path.basename(projDir)

sources = [
	f"src/**.cpp",
]

headers = [
	f"src/**.h",
]

includes = [
	projDir,
	f"src",
	f"{includeDirs['vendor']}",
	f"{includeDirs['glm']}",
	f"{includeDirs['stb_image']}",
	f"{includeDirs['yaml']}",
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
	f"{location}/modules/GUIModule/src",
]

sysIncludes = []
sysIncludes.extend(GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
	"EDITOR",
]

dependancys = [
	"GUIModule",
]

links = []

modules = [
	f"{location}/modules/GUIModule",
]

def GetProject():
	proj = BuildUtils.ProjectEnviernment()
	proj.projectDirectory = projDir
	proj.pchSource = ""
	proj.pchHeader = ""
	proj.sources = sources
	proj.headers = headers
	proj.includes = includes
	proj.sysIncludes = sysIncludes
	proj.defines = defines
	proj.links = links
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	proj.intDir = GetIntDir(projName, Config.project)
	proj.binDir = GetBinDir(projName, Config.project)
	proj.genReflection = True
	return proj


def GetModules():
	return modules
