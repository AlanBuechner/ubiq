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
		"src/*.h",
		"src/*.cpp",
		"src/Engine/**.h",
		"src/Engine/**.cpp",
		"src/Platform/Windows/**.h",
		"src/Platform/Windows/**.cpp",
		"src/Platform/DirectX12/**.h",
		"src/Platform/DirectX12/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/Glm/glm/**.hpp",
		"vendor/Glm/glm/**.inl",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"src",
		"vendor",
		"vendor/spdlog/include",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.Box2D}/Include",
		"%{IncludeDir.Assimp}/include",
		"%{IncludeDir.json}/single_include",
		"%{IncludeDir.dxc}/inc",
		"%{IncludeDir.pix}/include"
	}

	links 
	{ 
		"Glad",
		"ImGui",
		"Yaml",
		"Box2D",
		"%{IncludeDir.Assimp}/assimp-vc140-mt.lib",
		"%{IncludeDir.dxc}/lib/x64/dxcompiler.lib",
		"%{IncludeDir.pix}/bin/x64/WinPixEventRuntime.lib"
	}

	debugenvs
	{
		"PATH=%PATH%;%{IncludeDir.dxc}/bin/x64;%{IncludeDir.pix}/bin/x64"
	}

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

		defines
		{
			"BUILD_DLL",
			"GLFW_INCLUDE_NONE"
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
