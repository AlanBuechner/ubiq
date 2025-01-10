import os
location = os.path.dirname(__file__).replace("\\", "/")

projects = [] # will be populated later
buildScripts = {} # will be populated later
p = {
	"Dependancies" : {
		"imgui" : "vendor/imgui",
		"yaml-cpp" : "vendor/yaml-cpp",
		"ProjectManager" : "vendor/ProjectManager",
		"Reflection" : "vendor/Reflection",
		"Utilities" : "vendor/Utilities",
	},
	"GameEngine" : "GameEngine",
}
tools = [
	"tools/Socrates"
]

project = ""

configurations = [
	"Debug",
	"Release",
	"Dist",
]
configuration = configurations[1] # defult to release configuration

architectures = [
	"x86_64",
]
architecture = architectures[0]

systems = [
	"windows",
]
system = systems[0] # defult to windows

binDir = "bin/{config}-{system}-{arc}/{projName}"
intDir = "bin-int/{config}-{system}-{arc}/{projName}"

toolsBinDir = "bin/tools/{projName}"
toolsIntDir = "bin-int/tools/{projName}" 

cppVersion = "c++17"

compiler = f"{location}/vendor/Compiler/bin/clang-cl.exe"

vendorDirs = {
	# third party
	"yaml"				: f"{location}/vendor/yaml-cpp",
	"glm"				: f"{location}/vendor/glm",
	"ImGui"				: f"{location}/vendor/imgui",
	"stb_image"			: f"{location}/vendor/stb_image",
	"ImGuizmo"			: f"{location}/vendor/ImGuizmo",
	"Assimp"			: f"{location}/vendor/Assimp",
	"json"				: f"{location}/vendor/json",
	"dxc"				: f"{location}/vendor/dxc",
	"pix"				: f"{location}/vendor/pix",
	"Agility"			: f"{location}/vendor/Agility",
	
	# nont-third party
	"ProjectManager"	: f"{location}/vendor/ProjectManager",
	"Reflection"		: f"{location}/vendor/Reflection",
	"Utilities"			: f"{location}/vendor/Utilities",
}

includeDirs = {
	# third party
	"vendor"			: f"{location}/vendor",
	"yaml"				: f"{vendorDirs['yaml']}/include",
	"glm"				: f"{vendorDirs['glm']}",
	"ImGui"				: f"{vendorDirs['ImGui']}",
	"stb_image"			: f"{vendorDirs['stb_image']}",
	"ImGuizmo"			: f"{vendorDirs['ImGuizmo']}",
	"Assimp"			: f"{vendorDirs['Assimp']}/include",
	"json"				: f"{vendorDirs['json']}/single_include",
	"dxc"				: f"{vendorDirs['dxc']}/inc",
	"pix"				: f"{vendorDirs['pix']}/include",
	"Agility"			: f"{vendorDirs['Agility']}/build/native/include",

	# non-thrid party
	"ProjectManager"	: f"{vendorDirs['ProjectManager']}/src",
	"Reflection"		: f"{vendorDirs['Reflection']}/src",
	"Utilities"			: f"{vendorDirs['Utilities']}/src",
}

sysIncludeDirs = {

}

libs = {
	"dxc"				: f"{location}/vendor/dxc/lib/x64/dxcompiler.lib",
	"Assimp"			: f"{location}/vendor/Assimp/assimp-vc140-mt.lib",
	"pix"				: f"{location}/vendor/pix/bin/x64/WinPixEventRuntime.lib",
	"Aftermath"			: f"{location}/vendor/Aftermath/lib/x64/GFSDK_Aftermath_Lib.x64.lib",
	"Reflection"		: f"{location}/vendor/Compiler/lib/Reflection.lib",
	"ReflectionDebug"	: f"{location}/vendor/Compiler/lib/ReflectionD.lib"
}
