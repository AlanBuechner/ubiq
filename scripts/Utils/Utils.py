from scripts.Utils.Windows import *
import Config

import os

def GetBinDir(projName, root = None):
	if(root == None):
		root = Config.location
	return os.path.join(root, Config.binDir.format(config=Config.configuration, system=Config.system, arc=Config.architecture, projName=projName)).replace("\\", "/")

def GetIntDir(projName, root = None):
	if(root == None):
		root = Config.location
	return os.path.join(root, Config.intDir.format(config=Config.configuration, system=Config.system, arc=Config.architecture, projName=projName)).replace("\\", "/")

def GetToolsBinDir(projName, root = None):
	if(root == None):
		root = Config.location
	return os.path.join(root, Config.toolsBinDir.format(projName=projName)).replace("\\", "/")

def GetToolsIntDir(projName, root = None):
	if(root == None):
		root = Config.location
	return os.path.join(root, Config.toolsIntDir.format(projName=projName)).replace("\\", "/")

def ResolveFiles(filters, d):
	files = []
	for filt in filters:
		name, ext = os.path.splitext(os.path.basename(filt))
		folder = os.path.dirname(filt)
		if (folder != ""):
			folder += "/"
		if(not os.path.isabs(folder)):
			folder = d+"/"+folder

		if(name == "**"):
			for (dirpath, dirnames, filenames) in os.walk(folder):
				files.extend([dirpath + "/" + f for f in filenames if os.path.splitext(os.path.basename(f))[1] == ext])
		else:
			files.append(folder+name+ext)

	return files


def GetSysIncludes():
	includes = []
	includes.append(f"{Config.location}/vendor/Compiler/lib/clang/16/include")
	includes.append(f"{Config.location}/vendor/Compiler/include")
	includes.extend(GetWindowsIncludes())
	return includes


def GetTarget():
	if(Config.architecture == "x86_64"):
		if(Config.system == "windows"):
			return "x86_64-pc-windows-msvc"

def FindProjectPath(projName):
	for proj in Config.projects:
		if(proj.endswith(projName)):
			return proj
	print(f"project {projName} is not defined")
	return ""

def GetProject(projName):
	projPath = FindProjectPath(projName)
	return Config.buildScripts[projPath]
