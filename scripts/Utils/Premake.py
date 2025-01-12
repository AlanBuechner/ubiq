import Config
from scripts.Utils.Utils import *
from scripts.Utils.VSPatch import *
import subprocess

def AddProject(code, projectkey, projectvalue, group, indent):
	if(isinstance(projectvalue, dict)):
		newgorup = group + ("" if group == "" else "/") + projectkey
		code += f"group \"{newgorup}\"\n"
		for key, value in projectvalue.items():
			code = AddProject(code, key, value, newgorup, indent + 1)
		code += f"group \"{group}\"\n"
	else:
		code += ("\t" * indent) + f"include \"{Config.location}/{projectvalue}\"\n"
	return code


def GenerateSolution():
	gameName = os.path.basename(Config.project)
	code = f"""
workspace "{gameName}"
	architecture "x64"
	startproject "{Config.project}"

	configurations
	{{
"""
	# configurations
	for c in Config.configurations:
		code += f"\t\t\"{c}\",\n"
	code += "\t}\n\n"

	# ---- add include groups ---- #
	# tools group
	code += "group \"Tools\"\n"
	for t in Config.tools:
		code += f"\tinclude \"{Config.location}/{t}\"\n"
	code += "group \"\"\n"

	# groups defined in Config
	for key, value in Config.p.items():
		code = AddProject(code, key, value, "", 0)

	# modules
	code += "group \"Modules\"\n"
	for scriptName in Config.buildScripts:
		script = Config.buildScripts[scriptName]
		if(hasattr(script["module"], "GetModules")):
			for m in script["module"].GetModules():
				if(not os.path.isabs(m)):
					m = os.path.join(script["folder"], m).replace("\\", "/")
				code += f"\tinclude \"{m}\"\n"
	code += "group \"\"\n"

	return code

def GenerateProject(script, code = ""):
	proj = script["module"].GetProject()
	projName = os.path.basename(proj.projectDirectory)
	idir = proj.intDir
	bdir = proj.binDir

	gameName = os.path.basename(Config.project)
	if(projName == gameName):
				code += f"""
project "{projName}"
	kind "Makefile"

	targetdir ("{bdir}")
	objdir ("{idir}")

	buildcommands {{
		"\\"Build.bat\\" -fb -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {Config.project}"
	}}

	rebuildcommands  {{
		"\\"Build.bat\\" -rebuild -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {Config.project}"
	}}

	cleancommands  {{
		"\\"Build.bat\\"  -clean -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {Config.project}"
	}}

	files
	{{
"""
	else:
		code += f"""
project "{projName}"
	kind "Makefile"

	targetdir ("{bdir}")
	objdir ("{idir}")

	buildcommands {{
		"\\"{Config.location}/vendor/python/python.exe\\" {Config.location}/scripts/Build.py -fb -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {projName}"
	}}

	rebuildcommands  {{
		"\\"{Config.location}/vendor/python/python.exe\\" {Config.location}/scripts/Build.py -rebuild -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {projName}"
	}}

	cleancommands  {{
		"\\"{Config.location}/vendor/python/python.exe\\" {Config.location}/scripts/Build.py -clean -c %{{cfg.buildcfg}} -a %{{cfg.architecture}} -p {projName}"
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
	return code

def GenerateProjects():
	code = GenerateSolution()
	for script in Config.buildScripts.values():
		GenerateProject(script)

	projScript = Config.buildScripts[Config.project]
	GenerateProject(projScript, code)
	subprocess.run(["vendor\premake\premake5.exe", "vs2022"], cwd=Config.project)

	for script in Config.buildScripts.values():
		PatchVSXProj(script)
		GenVSXprojUser(script)

