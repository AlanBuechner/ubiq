project "Launcher"
	kind "ConsoleApp"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files { "src/**.h", "src/**.cpp" }

	includedirs
	{
		"src",

		"%{IncludeDir.ImGui}",
		"%{IncludeDir.GLFW}/include",
		"%{IncludeDir.glm}",
		"%{IncludeDir.ProjectManager}/src",

		"%{IncludeDir.Walnut}/src",

		"%{IncludeDir.VulkanSDK}",
	}

	links
	{
		"Walnut",
		"ProjectManager"
	}

	filter "system:windows"
		systemversion "latest"
		defines { "WL_PLATFORM_WINDOWS" }

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