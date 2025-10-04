# Reflection build script

import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None))
projName = os.path.basename(projDir)

sources = [
	"src/**.cpp"
]

headers = [
	"src/**.h"
]

includes = [
	f"{includeDirs['Utilities']}",
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
	f"Utilities"
]

def GetProject():
	proj = BuildUtils.ProjectEnviernment()
	proj.projectDirectory = projDir
	proj.pchSource = ""
	proj.pchHeader = ""
	proj.sources = sources
	proj.resources = []
	proj.headers = headers
	proj.includes = includes
	proj.sysIncludes = sysIncludes
	proj.defines = defines
	proj.links = []
	proj.dependancys = dependancys
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	proj.intDir = GetIntDir(projName, Config.project)
	proj.binDir = GetBinDir(projName, Config.project)
	return proj
