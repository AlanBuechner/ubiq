import xml.etree.ElementTree as ET
import os

def PatchVSXProj(script):
	proj = script["module"].GetProject()
	projName = os.path.basename(proj.projectDirectory)
	file = proj.projectDirectory + "/" + projName + ".vcxproj"

	print(f"patching {projName} VS project files")

	tree = ET.parse(file)
	ET.register_namespace("", "http://schemas.microsoft.com/developer/msbuild/2003")
	root = tree.getroot()

	xmlns = "{http://schemas.microsoft.com/developer/msbuild/2003}"

	includes = ""
	for inc in proj.includes:
		includes += f"{inc};"

	configurations = []
	for child in root.findall(f"{xmlns}ItemGroup"):
		if(child.get("Label") == "ProjectConfigurations"):
			for conf in child.findall(f"{xmlns}ProjectConfiguration"):
				configurations.append(conf.get("Include"))
	
	for child in root.findall(f"{xmlns}PropertyGroup"):
		if(not child.get("Label") and child.get("Condition")):
			defines = child.find(f"{xmlns}NMakePreprocessorDefinitions")
			if(not defines):
				defines = ET.SubElement(child, f"{xmlns}NMakePreprocessorDefinitions")
			
			# add defines
			defines.text = "STRIP_CLANG;"
			for d in proj.vsDefines:
				defines.text += f"{d};"

			# add include paths
			incs = child.find(f"{xmlns}NMakeIncludeSearchPath")
			if(not incs):
				incs = ET.SubElement(child, f"{xmlns}NMakeIncludeSearchPath")
			incs.text = includes

			options = child.find(f"{xmlns}AdditionalOptions")
			if(not options):
				options = ET.SubElement(child, f"{xmlns}AdditionalOptions")
			options.text = "/std:c++17"

	tree.write(file)

def GenVSXprojUser(script):
	proj = script["module"].GetProject()
	projName = os.path.basename(proj.projectDirectory)
	file = proj.projectDirectory + "/" + projName + ".vcxproj.user"

	print(f"generating {projName} VS project user file")

	with open(file, "w") as f:

		f.write(f"""<?xml version="1.0" encoding="utf-8"?>
<Project ToolsVersion="Current" xmlns="http://schemas.microsoft.com/developer/msbuild/2003">
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Debug|x64'">
    <LocalDebuggerCommand>{proj.projectDirectory}\\bin\\$(Configuration)-windows-x86_64\\Runtime\\Runtime.exe</LocalDebuggerCommand>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Dist|x64'">
    <LocalDebuggerCommand>{proj.projectDirectory}\\bin\\$(Configuration)-windows-x86_64\\Runtime\\Runtime.exe</LocalDebuggerCommand>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
  <PropertyGroup Condition="'$(Configuration)|$(Platform)'=='Release|x64'">
    <LocalDebuggerCommand>{proj.projectDirectory}\\bin\\$(Configuration)-windows-x86_64\\Runtime\\Runtime.exe</LocalDebuggerCommand>
    <DebuggerFlavor>WindowsLocalDebugger</DebuggerFlavor>
  </PropertyGroup>
</Project>
		""")


