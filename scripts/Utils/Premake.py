import Config
from scripts.Utils.Utils import *


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


def GenerateSolution():
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

def GenerateProject(script):
	proj = script["module"].GetProject()
	projName = os.path.basename(proj.projectDirectory)
	idir = proj.intDir
	bdir = proj.binDir
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
	code += "\t}\n\n"

	code += "\tlinks = \n\t{\n"
	for x in proj.dependancys:
		code += f"\t\t\"{x}\",\n"
	code += "\t}\n"

	f = open(proj.projectDirectory + "/premake5.lua", "w")
	f.write(code)
	f.close()

def GenerateProjects():
	GenerateSolution()
	for script in Config.buildScripts.values():
		GenerateProject(script)

	os.system("call vendor\premake\premake5.exe vs2022")
