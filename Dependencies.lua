VULKAN_SDK = os.getenv("VULKAN_SDK")
PROGRAM_FILES = os.getenv("ProgramW6432")

IncludeDir = {}
IncludeDir["stb_image"] = "%{wks.location}/KablunkEngine/vendor/stb_image"
IncludeDir["GLFW"] = "%{wks.location}/KablunkEngine/vendor/GLFW/include"
IncludeDir["Glad"] = "%{wks.location}/KablunkEngine/vendor/Glad/include"
IncludeDir["ImGui"] = "%{wks.location}/KablunkEngine/vendor/imgui"
IncludeDir["glm"] = "%{wks.location}/KablunkEngine/vendor/glm"
IncludeDir["entt"] = "%{wks.location}/KablunkEngine/vendor/entt/include"
IncludeDir["yaml_cpp"] = "%{wks.location}/KablunkEngine/vendor/yaml-cpp/include"
IncludeDir["spdlog"] = "%{wks.location}/KablunkEngine/vendor/spdlog/include"
IncludeDir["stduuid"] = "%{wks.location}/KablunkEngine/vendor/stduuid/include"
IncludeDir["gsl"] = "%{wks.location}/KablunkEngine/vendor/stduuid/gsl"
IncludeDir["ImGuizmo"] = "%{wks.location}/KablunkEngine/vendor/ImGuizmo"
IncludeDir["assimp"] = "%{wks.location}/KablunkEngine/vendor/assimp/include"
IncludeDir["FreeType"] = "%{wks.location}/KablunkEngine/vendor/FreeType/include"
IncludeDir["Box2d"] = "%{wks.location}/KablunkEngine/vendor/box2d/include"
IncludeDir["cr"] = "%{wks.location}/KablunkEngine/vendor/cr"
IncludeDir["mono"] = "%{wks.location}/KablunkEngine/vendor/mono/include"
IncludeDir["VulkanSDK"] = "%{VULKAN_SDK}/Include"
IncludeDir["VulkanSDK_LocalInclude"] = "%{wks.location}/KablunkEngine/vendor/VulkanSDK/Include"
IncludeDir["boost"] = "%{PROGRAM_FILES}/boost"
IncludeDir["miniaudio"] = "%{wks.location}/KablunkEngine/vendor/miniaudio/include"
IncludeDir["optick"] = "%{wks.location}/KablunkEngine/vendor/optick/src"
IncludeDir["tl_expected"] = "%{wks.location}/KablunkEngine/vendor/expected/include"
IncludeDir["robin_hood"] = "%{wks.location}/KablunkEngine/vendor/robin-hood/include"
IncludeDir["flatbuffers"] = "%{wks.location}/KablunkEngine/vendor/flatbuffers/include"

LibraryDir = {}
LibraryDir["VulkanSDK"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Debug"] = "%{VULKAN_SDK}/Lib"
LibraryDir["VulkanSDK_Bin"] = "%{VULKAN_SDK}/Bin"
LibraryDir["VulkanSDK_Debug_Bin"] = "%{VULKAN_SDK}/Bin"

Library = {}
Library["mono"] = "%{wks.location}/KablunkEngine/vendor/mono/lib/Debug/mono-2.0-sgen.lib"
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
