project "USG-Editor"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	toolset "clang"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.h",
		"src/**.cpp",
		"embeded/**.rc",
	}

	includedirs
	{
		"src",
		"%{wks.location}/vendor",
		"%{wks.location}/GameEngine/vendor",
		"%{wks.location}/GameEngine/src",
		"%{wks.location}/GameEngine/embeded",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ProjectManager}/src"
	}

	sysincludedirs 
	{
		"$(SolutionDir)vendor\\Compiler\\include"
	}

	links
	{
		"GameEngine",
		"ProjectManager"
	}

	filter "system:windows"
		systemversion "latest"

		defines
		{
		}

	filter "configurations:Debug"
		defines "DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Dist"
		defines "DIST"
		runtime "Release"
		optimize "on"
