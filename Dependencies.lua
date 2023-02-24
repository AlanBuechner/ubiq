VULKAN_SDK = os.getenv("VULKAN_SDK")

if(VULKAN_SDK == nil) then
    print("ERROR: could not find Vulkan SDK")
end


-- Include directories relative to root folder (solution directory)
IncludeDir = {}
-- general includes
IncludeDir["yaml"] = "%{wks.location}/vendor/yaml-cpp/include"
IncludeDir["glm"] = "%{wks.location}/vendor/glm"
IncludeDir["ImGui"] = "%{wks.location}/vendor/imgui"
IncludeDir["stb_image"] = "%{wks.location}/vendor/stb_image"
IncludeDir["GLFW"] = "%{wks.location}/vendor/GLFW"
IncludeDir["Walnut"] = "%{wks.location}/vendor/Walnut"
IncludeDir["ProjectManager"] = "%{wks.location}/vendor/ProjectManager"
IncludeDir["Compiler"] = "%{wks.location}/vendor/Compiler"

-- engine includes
IncludeDir["ImGuizmo"] = "%{wks.location}/GameEngine/vendor/ImGuizmo"
IncludeDir["Box2D"] = "%{wks.location}/GameEngine/vendor/Box2D"
IncludeDir["Assimp"] = "%{wks.location}/GameEngine/vendor/Assimp"
IncludeDir["json"] = "%{wks.location}/GameEngine/vendor/json"
IncludeDir["dxc"] = "%{wks.location}/GameEngine/vendor/dxc"
IncludeDir["pix"] = "%{wks.location}/GameEngine/vendor/pix"


-- sdks
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"

Library = {}
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"