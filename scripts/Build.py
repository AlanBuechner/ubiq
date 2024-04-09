# main build file

import sys
import os
import inspect
import argparse
import time
import importlib

# set our working directory to the projects root directory
rootDir = os.path.dirname(os.path.dirname(inspect.getfile(lambda: None)))
os.chdir(rootDir)
sys.path.append(rootDir)

import Config
import scripts.BuildUtils as BuildUtils
from scripts.Utils.Utils import *
import scripts.Utils.Premake as Premake

# --------------------- Setup Projects --------------------- #
def CollapseProject(projectkey, projectvalue):
	if(isinstance(projectvalue, dict)):
		for key, value in projectvalue.items():
			CollapseProject(key, value)
	else:
		Config.projects.append(projectvalue)

# collapse p dictinary into a list of projects
for key, value in Config.p.items():
	CollapseProject(key, value) 


# --------------------- Define args --------------------- #
parser = argparse.ArgumentParser()
parser.add_argument('-gs', action='store_true', help='generate project files') # generate project files
parser.add_argument('-r', action='store_true', help='run the project') # run
parser.add_argument('-b', action='store_true', help='build the project') # build
parser.add_argument('-fb', action='store_true', help='build the full project') # full build
parser.add_argument('-p', type=str, help='the project to build') # project
parser.add_argument('-c', type=str, help='the configuration (Release, Debug, Dist)') # configuration
parser.add_argument('-a', type=str, help='the architecture (x64)') # architecture
parser.add_argument('-s', type=str, help='the system (windows)') # system
args = parser.parse_args()

# --------------------- Check args --------------------- #
shouldBuild = args.b
shouldRun = args.r
fullbuild = args.fb
if(fullbuild):
	shouldBuild = True

# get the project
buildProject = ""
if(args.p != None):
	buildProject = FindProject(args.p)
	if(buildProject == ""):
		exit(1)

def checkArg(arg, options):
	for option in options:
		if(arg == option):
			return option
	return ""

# get the configuration
if(args.c != None):
	c = checkArg(args.c, Config.configurations)
	if(c == ""):
		print(f"configuration {args.c} is not defined")
		exit(1)
	else:
		Config.configuration = c

# get the architecture
if(args.a != None):
	a = checkArg(args.a, Config.architectures)
	if(a == ""):
		print(f"architecture {args.a} is not defined")
		exit(1)
	else:
		Config.architecture = a

# get the system
if(args.s != None):
	s = checkArg(args.s, Config.systems)
	if(s == ""):
		print(f"system {args.s} is not defined")
		exit(1)
	else:
		Config.system = s

# --------------------- Load Project Scritps ---------------------
for proj in Config.projects:
	module = importlib.import_module(proj.replace("/", ".") + ".Build")
	Config.buildScripts[proj] = {
		"module" : module,
		"built" : False,
		"folder" : proj
	}

# --------------------- Generate Project Files --------------------- #
if(args.gs):
	Premake.GenerateProjects()

# --------------------- Build Project --------------------- #
def BuildProject(proj):
	# return if project has already been built
	if(proj == "" or Config.buildScripts[proj]["built"]):
		return

	# build projects depandancies if full build is enabled
	if(fullbuild):
		for d in Config.buildScripts[proj]["module"].GetProject().dependancys:
			BuildProject(FindProject(d))

	# build project
	print("Building " + proj)
	startTime = time.time()
	errorCode = Config.buildScripts[proj]["module"].GetProject().Build()
	Config.buildScripts[proj]["built"] = True
	endTime = time.time()
	if(errorCode != 0):
		print("Build Failed")
	print(f"finished building {proj} in {endTime - startTime}s")
	return errorCode

if(shouldBuild):
	if(buildProject == ""):
		fullbuild = True
		for proj in Config.projects:
			BuildProject(proj)
	else:
		BuildProject(buildProject)

# --------------------- Run Project --------------------- #
def RunProject(projName):
	proj = GetProject(projName)["module"].GetProject()
	bdir = proj.binDir
	os.startfile(proj.GetOutput(), cwd = os.path.normpath(proj["folder"]))

if(shouldRun):
	if(buildProject == ""):
		RunProject(Config.startupProject.split("/")[-1])
	else:
		RunProject(buildProject.split("/")[-1])
