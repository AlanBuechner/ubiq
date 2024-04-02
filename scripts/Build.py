# main build file

import sys
import os
import inspect
rootDir = os.path.dirname(os.path.dirname(inspect.getfile(lambda: None)))
os.chdir(rootDir)
sys.path.append(rootDir)

import argparse
import time
import importlib
import Config
import scripts.BuildUtils as BuildUtils
os.system("color")

parser = argparse.ArgumentParser()
parser.add_argument('-r', action='store_true', help='run the project') # run
parser.add_argument('-b', action='store_true', help='build the project') # build
parser.add_argument('-br', action='store_true', help='build and run the project') # build and run
parser.add_argument('-p', type=str, help='the project to build') # project
parser.add_argument('-c', type=str, help='the configuration (Release, Debug, Dist)') # configuration
parser.add_argument('-a', type=str, help='the architecture (x64)') # architecture
parser.add_argument('-s', type=str, help='the system (windows)') # system
args = parser.parse_args()

shouldBuild = True
shouldRun = False

if(args.r):
	shouldBuild = False
	shouldRun = True

if(args.br):
	shouldBuild = True
	shouldRun = True

buildProject = ""
if(args.p != None):
	buildProject = BuildUtils.FindProject(args.p)
	if(buildProject == ""):
		exit()

def checkArg(arg, options):
	for option in options:
		if(arg == option):
			return option
	return ""

if(args.c != None):
	c = checkArg(args.c, Config.configurations)
	if(c == ""):
		print(f"configuration {args.c} is not defined")
		exit()
	else:
		Config.configuration = c

if(args.a != None):
	a = checkArg(args.a, Config.architectures)
	if(a == ""):
		print(f"architecture {args.a} is not defined")
		exit()
	else:
		Config.architecture = a

if(args.s != None):
	s = checkArg(args.s, Config.systems)
	if(s == ""):
		print(f"system {args.s} is not defined")
		exit()
	else:
		Config.system = s

buildScripts = {}
for proj in Config.projects:
	module = importlib.import_module(proj.replace("/", ".") + ".Build")
	buildScripts[proj] = {
		"module" : module,
		"built" : False,
	}

def BuildProject(proj):
	# return if project has already been built
	if(proj == "" or buildScripts[proj]["built"]):
		return

	for d in buildScripts[proj]["module"].GetProject().dependancys:
		BuildProject(BuildUtils.FindProject(d))

	print("Building " + proj)
	startTime = time.time()
	errorCode = buildScripts[proj]["module"].GetProject().Build()
	buildScripts[proj]["built"] = True
	endTime = time.time()
	if(errorCode != 0):
		print("Build Failed")
	print(f"finished building {proj} in {endTime - startTime}s")
	return errorCode

def BuildAllProjects():
	for proj in Config.projects:
		BuildProject(proj)

def RunProject(proj):
	bdir = BuildUtils.GetBinDir(proj)
	exe = bdir+"/"+proj+".exe"
	os.startfile(exe, cwd = os.path.normpath(BuildUtils.FindProject(proj)))

if(shouldBuild):
	if(buildProject == ""):
		BuildAllProjects()
	else:
		BuildProject(buildProject)

if(shouldRun):
	if(buildProject == ""):
		RunProject(Config.projects[0].split("/")[-1])
	else:
		RunProject(buildProject.split("/")[-1])
