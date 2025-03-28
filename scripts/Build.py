# main build file

import sys
import os
import shutil
import inspect
import argparse
import time
import importlib.util
import subprocess

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
	# check if value is a dict or a string
	if(isinstance(projectvalue, dict)):
		# add projects recursivly
		for key, value in projectvalue.items():
			CollapseProject(key, value)
	else:
		# append project path
		Config.projects.append(os.path.join(Config.location, projectvalue))

# collapse p dictinary into a list of projects
for key, value in Config.p.items():
	CollapseProject(key, value)
Config.projects.extend(Config.tools)


# --------------------- Define args --------------------- #
parser = argparse.ArgumentParser()
parser.add_argument('-gs', action='store_true', help='generate project files') # generate project files
parser.add_argument('-r', action='store_true', help='run the project') # run
parser.add_argument('-b', action='store_true', help='build the project') # build
parser.add_argument('-fb', action='store_true', help='build the full project') # full build
parser.add_argument('-rebuild', action='store_true', help='rebuild the project') # rebuild the project
parser.add_argument('-clean', action='store_true', help='clean the project') # clean build files
parser.add_argument('--BuildTools', action='store_true', help='build tools') # build tools
parser.add_argument('-p', type=str, help='the project to build') # project
parser.add_argument('-c', type=str, help='the configuration (Release, Debug, Dist)') # configuration
parser.add_argument('-a', type=str, help='the architecture (x64)') # architecture
parser.add_argument('-s', type=str, help='the system (windows)') # system
args = parser.parse_args()

# --------------------- Check args --------------------- #
shouldBuild = args.b or args.rebuild or args.fb
shouldRun = args.r
fullbuild = args.fb
clean = args.clean or args.rebuild
if(fullbuild):
	shouldBuild = True

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

# get the project
if(args.p != None):
	Config.project = args.p.replace("\\", "/")
	if(Config.project[-1] == "/"):
		Config.project = Config.project[:-1]
	Config.project = os.path.abspath(Config.project).replace("\\", "/")
	Config.projects.append(Config.project)


# --------------------- Build Tools --------------------- #
if(args.BuildTools):
	# overide configurations for build tools
	 Config.configuration = "Release"
	 Config.architecture = "x86_64"
	 Config.system = "windows"
elif(shouldBuild):
	# validate build tools have been built
	a = ["vendor/python/python.exe", "scripts/Build.py", "--BuildTools"]
	result = subprocess.run(a)

# --------------------- Load Project Scritps --------------------- #
def LoadProject(proj):
	# load project scritps
	projName = os.path.basename(proj)
	spec=importlib.util.spec_from_file_location(projName, proj+"/Build.py")
	module = importlib.util.module_from_spec(spec)
	spec.loader.exec_module(module)
	Config.buildScripts[proj] = {
		"module" : module,
		"built" : False,
		"folder" : proj
	}

	# load project modules
	if(hasattr(module, "GetModules")):
		for m in module.GetModules():
			# get abspath of module
			modulePath = m
			if(not os.path.isabs(modulePath)):
				modulePath = os.path.join(proj, modulePath).replace("\\", "/")
			# check if module exists
			if(modulePath in Config.projects):
				continue
			Config.projects.append(modulePath) # append new project
			LoadProject(modulePath) # load new project




# load project
for proj in Config.projects:
	LoadProject(proj)



# --------------------- Generate Project Files --------------------- #
if(args.gs):
	Premake.GenerateProjects()

# --------------------- Clean Projects --------------------- #

def CleanProject(projName):
	proj = Config.buildScripts[projName]["module"].GetProject()
	shutil.rmtree(proj.intDir, ignore_errors=True)

if(clean):
	if(Config.project == ""):
		fullbuild = True
		for	proj in Config.projects:
			CleanProject(proj)
	else:
		CleanProject(Config.project)

# --------------------- Build Projects --------------------- #
def BuildProject(proj):
	# return if project has already been built
	if(proj == "" or Config.buildScripts[proj]["built"]):
		return

	# build projects depandancies if full build is enabled
	if(fullbuild):
		for d in Config.buildScripts[proj]["module"].GetProject().dependancys:
			BuildProject(FindProjectPath(d))

	# build project
	buildMsg = f"=============== Building: {proj},   {Config.configuration} {Config.architecture} ==============="
	print("_" * 120)
	print(buildMsg)
	startTime = time.time()
	errorCode = Config.buildScripts[proj]["module"].GetProject().Build()
	Config.buildScripts[proj]["built"] = True
	endTime = time.time()
	if(errorCode != 0):
		print("Build Failed")
	print(f"========== finished building {proj} in {endTime - startTime:.2f}s ==========")
	print("_" * 120)
	print("\n")
	return errorCode


if(args.BuildTools):
	for t in Config.tools:
		BuildProject(t)

if(shouldBuild):
	if(Config.project == ""):
		fullbuild = True
		for	proj in Config.projects:
			BuildProject(proj)
	else:
		BuildProject(Config.project)

# --------------------- Run Project --------------------- #
def RunProject(projName):
	proj = GetProject(projName)
	projdata = GetProject(projName)["module"].GetProject()
	output = projdata.GetOutput()
	cwd = Config.project
	os.startfile(output, cwd = cwd)

if(shouldRun):
	RunProject(Config.project.split("/")[-1])
