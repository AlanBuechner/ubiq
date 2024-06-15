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
			defines.text = "STRIP_CLANG"

			incs = child.find(f"{xmlns}AdditionalOptions")
			if(not incs):
				incs = ET.SubElement(child, f"{xmlns}NMakeIncludeSearchPath")
			incs.text = includes

			options = child.find(f"{xmlns}NMakeIncludeSearchPath")
			if(not options):
				options = ET.SubElement(child, f"{xmlns}AdditionalOptions")
			options.text = "/std:c++17"

	tree.write(file);
