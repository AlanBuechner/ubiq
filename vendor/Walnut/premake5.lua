project "Walnut"
   kind "StaticLib"
   language "C++"
   cppdialect "C++17"
   targetdir "bin/%{cfg.buildcfg}"
   staticruntime "on"

   files { "src/**.h", "src/**.cpp" }

   includedirs
   {
      "src",

      "%{IncludeDir.ImGui}",
      "%{IncludeDir.GLFW}/include",
      "%{IncludeDir.stb_image}",

      "%{IncludeDir.VulkanSDK}",
      "%{IncludeDir.glm}",
   }

   links
   {
       "ImGui",
       "GLFW",

       "%{Library.Vulkan}",
   }

   targetdir ("bin/" .. outputdir .. "/%{prj.name}")
   objdir ("../bin-int/" .. outputdir .. "/%{prj.name}")

   filter "system:windows"
      systemversion "latest"
      defines { "WL_PLATFORM_WINDOWS" }

   filter "configurations:Debug"
      defines { "WL_DEBUG" }
      runtime "Debug"
      symbols "On"

   filter "configurations:Release"
      defines { "WL_RELEASE" }
      runtime "Release"
      optimize "On"

   filter "configurations:Dist"
      defines { "WL_DIST" }
      runtime "Release"
      optimize "On"