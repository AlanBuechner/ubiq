project "Sandbox"
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
		"src/**.cpp"
	}

	includedirs
	{
		"src",
		"%{wks.location}/GameEngine/vendor/spdlog/include",
		"%{wks.location}/GameEngine/vendor",
		"%{wks.location}/GameEngine/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}"
	}

	sysincludedirs 
	{
		"$(SolutionDir)vendor\\Compiler\\include"
	}

	links
	{
		"GameEngine"
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
