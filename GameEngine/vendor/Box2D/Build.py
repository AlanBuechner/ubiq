# yaml build script

import scripts.BuildUtils as BuildUtils
from Config import *
import time

import inspect
projDir = os.path.dirname(inspect.getfile(lambda: None))
projName = os.path.basename(projDir)

sources = [
	"src/**.cpp",
]

headers = [
	"src/**.h",
	"include/**.h"
]

includes = [
	"src",
	"include"
]

sysIncludes = []
sysIncludes.extend(BuildUtils.GetSysIncludes())

defines = [
	configuration.upper(),
	"_UNICODE",
	"UNICODE",
	"_CRT_SECURE_NO_WARNINGS",
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
	proj.buildType = BuildUtils.BuildType.STATICLIBRARY
	return proj

