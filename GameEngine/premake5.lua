project "GameEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	toolset "clang"
	staticruntime "on"
	openmp "On"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "pch.h"
	pchsource "src/pch.cpp"

	files
	{
		"generated/generated.obj",
		"src/**.h",
		"src/**.cpp",
		"%{IncludeDir.stb_image}/**.h",
		"%{IncludeDir.stb_image}/**.cpp",
		"%{IncludeDir.ImGuizmo}/ImGuizmo.h",
		"%{IncludeDir.ImGuizmo}/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Box2D}/Include",
		"%{IncludeDir.Assimp}/include",
		"%{IncludeDir.json}/single_include",
		"%{IncludeDir.dxc}/inc",
		"%{IncludeDir.pix}/include"
	}

	sysincludedirs 
	{
		"%{IncludeDir.Compiler}/include"
	}

	links 
	{
		"ImGui",
		"Yaml",
		"Box2D",
		"%{IncludeDir.Assimp}/assimp-vc140-mt.lib",
		"%{IncludeDir.dxc}/lib/x64/dxcompiler.lib",
		"%{IncludeDir.pix}/bin/x64/WinPixEventRuntime.lib"
	}

	debugenvs
	{
		"PATH=%PATH%;%{IncludeDir.dxc}bin/x64;%{IncludeDir.pix}bin/x64"
	}

	prelinkcommands
	{
		--"%{wks.location}vendor\\Compiler\\bin\\Socrates.exe \"$(ProjectDir)\\\" \"$(IncludePath)\" \"$(IntermediateOutputPath)\\\""
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "on"

		links
		{
			"%{IncludeDir.Compiler}/lib/ReflectionD.lib"
		}

	filter "configurations:Release"
		defines "RELEASE"
		runtime "Release"
		optimize "on"

		links
		{
			"%{IncludeDir.Compiler}/lib/Reflection.lib"
		}

	filter "configurations:Dist"
		defines "DIST"
		runtime "Release"
		optimize "on"

		links
		{
			"%{IncludeDir.Compiler}/lib/Reflection.lib"
		}
