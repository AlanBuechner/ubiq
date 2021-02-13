project "Yaml"
    kind "StaticLib"
    language "C++"
    cppdialect "C++17"
    staticruntime "On"
    
    targetdir ("bin/" .. outputdir .. "/%{prj.name}")
    objdir ("bin-int/" .. outputdir .. "/%{prj.name}")
    
    files
	{
		"src/**.h",
		"src/**.cpp",
		
		"include/**.h"
	}
    
    includedirs
    {
        "include"
	}

    filter "system:windows"
        systemversion "latest"
        staticruntime "On"
    
    filter "configurations:Debug"
    	runtime "Debug"
    	symbols "on"
    
    filter "configurations:Release"
    	runtime "Release"
    	optimize "on"
