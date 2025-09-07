# test module build script

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
	f"{includeDirs['glm']}",
	f"{includeDirs['stb_image']}",
	f"{includeDirs['Reflection']}",
	f"{includeDirs['ProjectManager']}",
	f"{includeDirs['Utilities']}",
	f"{includeDirs['tracy']}",
	f"{location}/GameEngine/src",
	f"{location}/GameEngine/embeded",
]

sysIncludes = []
sysIncludes.extend(GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
]

dependancys = []

links = []

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
	proj.links = links
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	proj.intDir = GetIntDir(projName, Config.project)
	proj.binDir = GetBinDir(projName, Config.project) 
	proj.genReflection = True
	return proj



