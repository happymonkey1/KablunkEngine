VULKAN_SDK = os.getenv("VULKAN_SDK")
KablunkRootDirectory = os.getenv("KABLUNK_DIR")
PROGRAM_FILES = os.getenv("ProgramW6432")

IncludeDir = {}
IncludeDir["stb_image"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/stb_image"
IncludeDir["GLFW"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/imgui"
IncludeDir["glm"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/glm"
IncludeDir["entt"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/yaml-cpp/include"
IncludeDir["spdlog"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/spdlog/include"
IncludeDir["stduuid"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/stduuid/include"
IncludeDir["gsl"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/stduuid/gsl"
IncludeDir["ImGuizmo"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/ImGuizmo"
IncludeDir["assimp"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/assimp/include"
IncludeDir["FreeType"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/FreeType/include"
IncludeDir["Box2d"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/box2d/include"
IncludeDir["cr"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/cr"
IncludeDir["mono"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/mono/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["VulkanSDK_LocalInclude"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/VulkanSDK/Include"
IncludeDir["boost"] = "%{PROGRAM_FILES}/boost"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Bin"] = "%{VULKAN_SDK}/Bin"
LibraryDir["VulkanSDK_Debug_Bin"] = "%{VULKAN_SDK}/Bin"
LibraryDir["KablunkSDK"] = "%{KablunkRootDirectory}/bin"

Library = {}
Library["mono"] = "%{KablunkRootDirectory}/KablunkEngine/vendor/mono/lib/Debug/mono-2.0-sgen.lib"
Library["Vulkan"] = "%{LibraryDir.VulkanSDK}/vulkan-1.lib"
Library["VulkanUtils"] = "%{LibraryDir.VulkanSDK}/VkLayer_utils.lib"

Library["ShaderC_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_sharedd.lib"
Library["ShaderC_Utils_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/shaderc_utild.lib"
Library["SPIRV_Cross_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-cored.lib"
Library["SPIRV_Cross_GLSL_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/spirv-cross-glsld.lib"
Library["SPIRV_Tools_Debug"] = "%{LibraryDir.VulkanSDK_Debug}/SPIRV-Toolsd.lib"

Library["ShaderC_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_shared.lib"
Library["ShaderC_Utils_Release"] = "%{LibraryDir.VulkanSDK}/shaderc_util.lib"
Library["SPIRV_Cross_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-core.lib"
Library["SPIRV_Cross_GLSL_Release"] = "%{LibraryDir.VulkanSDK}/spirv-cross-glsl.lib"
Library["SPIRV_Tools"] = "%{LibraryDir.VulkanSDK}/SPIRV-Tools.lib"

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"
Library["Kablunk"] = "%{LibraryDir.KablunkSDK}/" .. outputdir .. "/KablunkEngine/KablunkEngine.lib"
