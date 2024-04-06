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
parser.add_argument('-gs', action='store_true', help='generate project files') # generate project files
parser.add_argument('-r', action='store_true', help='run the project') # run
parser.add_argument('-b', action='store_true', help='build the project') # build
parser.add_argument('-fb', action='store_true', help='build the full project') # full build
parser.add_argument('-p', type=str, help='the project to build') # project
parser.add_argument('-c', type=str, help='the configuration (Release, Debug, Dist)') # configuration
parser.add_argument('-a', type=str, help='the architecture (x64)') # architecture
parser.add_argument('-s', type=str, help='the system (windows)') # system
args = parser.parse_args()

def CollapseProject(projectkey, projectvalue):
	if(isinstance(projectvalue, dict)):
		for key, value in projectvalue.items():
			CollapseProject(key, value)
	else:
		Config.projects.append(projectvalue)

for key, value in Config.p.items():
		CollapseProject(key, value) 

buildScripts = {}
for proj in Config.projects:
	module = importlib.import_module(proj.replace("/", ".") + ".Build")
	buildScripts[proj] = {
		"module" : module,
		"built" : False,
		"folder" : proj
	}

def GenerateProjects():
	code = f"""
workspace "UbiqEngine"
	architecture "x64"
	startproject "{Config.startupProject}"

	configurations
	{{
"""
	for c in Config.configurations:
		code += f"\t\t\"{c}\",\n"
	code += "\t}\n\n"

	for key, value in Config.p.items():
		code = AddProject(code, key, value, "", 0)

	f=open("premake5.lua", "w")
	f.write(code)
	f.close()

	for script in buildScripts.values():
		proj = script["module"].GetProject()
		projName = os.path.basename(proj.projectDirectory)
		idir = BuildUtils.GetIntDir(projName).replace("\\", "/")
		bdir = BuildUtils.GetBinDir(projName).replace("\\", "/")
		code = f"""
project "{projName}"
	kind "Makefile"

	targetdir ("{bdir}")
	objdir ("{idir}")

	buildcommands {{
		"\\"{Config.location}/vendor/python/python.exe\\" {Config.location}/scripts/Build.py -b -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {projName}"
	}}

	files
	{{
"""
		for x in proj.sources:
			code += f"\t\t\"{x}\",\n"
		for x in proj.headers:
			code += f"\t\t\"{x}\",\n"
		for x in proj.resources:
			code += f"\t\t\"{x}\",\n"

		code += "\t}\n\n"
		code += "\tincludedirs = \n\t{\n"
		for x in proj.includes:
			code += f"\t\t\"{x}\",\n"
		code += "\t}\n\n"
		code += "\tsysincludedirs  = \n\t{\n"
		for x in proj.sysIncludes:
			code += f"\t\t\"{x}\",\n"
		code += "\t}\n"

		code += "\tlinks = \n\t{\n"
		for x in proj.dependancys:
			code += f"\t\t\"{x}\",\n"
		code += "\t}\n"

		f = open(proj.projectDirectory + "/premake5.lua", "w")
		f.write(code)
		f.close()

	os.system("call vendor\premake\premake5.exe vs2022")

def AddProject(code, projectkey, projectvalue, group, indent):
	if(isinstance(projectvalue, dict)):
		newgorup = group + ("" if group == "" else "/") + projectkey
		code += f"group \"{newgorup}\"\n"
		for key, value in projectvalue.items():
			code = AddProject(code, key, value, newgorup, indent + 1)
		code += f"group \"{group}\"\n"
	else:
		code += ("\t" * indent) + f"include \"{projectvalue}\"\n"
	return code

if(args.gs):
	GenerateProjects()
	exit()

shouldBuild = args.b
shouldRun = args.r
fullbuild = args.fb

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

def BuildProject(proj):
	# return if project has already been built
	if(proj == "" or buildScripts[proj]["built"]):
		return

	if(fullbuild):
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
		RunProject(Config.startupProject.split("/")[-1])
	else:
		RunProject(buildProject.split("/")[-1])
