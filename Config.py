import os
location = os.path.dirname(__file__).replace("\\", "/")
vulkanSDK = os.getenv("VULKAN_SDK")

projects = [] # will be populated by the p dictinary
p = {
	"Dependancies" : {
		"imgui" : "vendor/imgui",
		"yaml-cpp" : "vendor/yaml-cpp",
		"ProjectManager" : "vendor/ProjectManager",
	},
	"USG-Editor" : "USG-Editor",
	"GameEngine" : "GameEngine",
}

startupProject = "USG-Editor"

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

outDir = "bin/{config}-{system}-{arc}/{projName}"
intDir = "bin-int/{config}-{system}-{arc}/{projName}"

cppVersion = "c++17"

openMP = True

compiler = f"{location}/vendor/Compiler/bin/clang-cl.exe"
reflector =  f"{location}/vendor/Compiler/bin/Socrates.exe"

includeDirs = {
	# general includes
	"vendor"			: f"{location}/vendor",
	"yaml"				: f"{location}/vendor/yaml-cpp/include",
	"glm"				: f"{location}/vendor/glm",
	"ImGui"				: f"{location}/vendor/imgui",
	"stb_image"			: f"{location}/vendor/stb_image",
	"ProjectManager"	: f"{location}/vendor/ProjectManager",

	# engine includes
	"ImGuizmo"			: f"{location}/GameEngine/vendor/ImGuizmo",
	"Assimp"			: f"{location}/GameEngine/vendor/Assimp/include",
	"json"				: f"{location}/GameEngine/vendor/json/single_include",
	"dxc"				: f"{location}/GameEngine/vendor/dxc/inc",
	"pix"				: f"{location}/GameEngine/vendor/pix/include",
	"Aftermath"			: f"{location}/GameEngine/vendor/Aftermath/include",
}

sysIncludeDirs = {
	"Compiler"			: f"{location}/vendor/Compiler/include"
}

libs = {
	"dxc"				: f"{location}/GameEngine/vendor/dxc/lib/x64/dxcompiler.lib",
	"Assimp"			: f"{location}/GameEngine/vendor/Assimp/assimp-vc140-mt.lib",
	"pix"				: f"{location}/GameEngine/vendor/pix/bin/x64/WinPixEventRuntime.lib",
	"Aftermath"			: f"{location}/GameEngine/vendor/Aftermath/lib/x64/GFSDK_Aftermath_Lib.x64.lib",
	"Reflection"		: f"{location}/vendor/Compiler/lib/Reflection.lib",
	"ReflectionDebug"	: f"{location}/vendor/Compiler/lib/ReflectionD.lib"
}