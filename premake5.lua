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
	include "GameEngine/vendor/Glad"
	include "GameEngine/vendor/imgui"
	include "GameEngine/vendor/yaml-cpp"
	include "GameEngine/vendor/Box2D"
group ""


include "GameEngine"
include "Sandbox"
include "USG-Editor"
