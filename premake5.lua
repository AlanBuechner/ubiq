include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "UbiqEngine"
	architecture "x64"
	startproject "USG-Editor"

	configurations
	{
		"Debug",
		"Release",
		"Dist"
	}

	solution_items
	{
		".editorconfig"
	}

	flags
	{
		"MultiProcessorCompile"
	}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
	include "vendor/Walnut"
	include "vendor/imgui"
	include "vendor/GLFW"
	include "vendor/yaml-cpp"
	include "vendor/ProjectManager"
	include "GameEngine/vendor/Box2D"
group ""

include "GameEngine"
include "Sandbox"
include "USG-Editor"
include "Launcher"

