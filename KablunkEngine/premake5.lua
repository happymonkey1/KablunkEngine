project "KablunkEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "kablunkpch.h"
	pchsource "src/kablunkpch.cpp"

	files
	{
		"include/**.h",
		"src/**.cpp",
		"vendor/stb_image/**.h",
		"vendor/stb_image/**.cpp",
		"vendor/glm/glm/**.hpp",
		"vendor/glm/glm/**.inl",
		"vendor/stduuid/include/uuid.h",
		"vendor/ImGuizmo/ImGuizmo.h",
		"vendor/ImGuizmo/ImGuizmo.cpp",

		"vendor/VulkanMemoryAllocator/vk_mem_alloc.h",
		"vendor/VulkanMemoryAllocator/vk_mem_alloc.cpp",
		"vendor/cr/cr.h"

	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"NOMINMAX",
		"KB_BUILD_DLL"
	}

	includedirs
	{
		"include",
		"src",
		"vendor/spdlog/include",
		"%{IncludeDir.GLFW}",
		"%{IncludeDir.Glad}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.glm}",
		"%{IncludeDir.stb_image}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.yaml_cpp}",
		"%{IncludeDir.spdlog}",
		"%{IncludeDir.stduuid}",
		"%{IncludeDir.gsl}",
		"%{IncludeDir.ImGuizmo}",
		"%{IncludeDir.assimp}",
		"%{IncludeDir.FreeType}",
		"%{IncludeDir.Box2d}",
		"%{IncludeDir.cr}",
		"%{IncludeDir.mono}",
		"%{IncludeDir.Vulkan}",
		"%{IncludeDir.VulkanSDK}",
		"%{IncludeDir.VulkanSDK_LocalInclude}"
	}

	links
	{
		"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"spdlog",
		"FreeType",
		"Box2d",
		"opengl32.lib",

		"%{Library.Vulkan}",
		"%{Library.VulkanUtils}",

		"%{Library.mono}"
	}
	

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "KB_DEBUG"
		runtime "Debug"
		symbols "on"

		links
        {
            "vendor/assimp/bin/Debug/assimp-vc141-mtd.lib",
			"%{Library.ShaderC_Debug}",
			"%{Library.ShaderC_Utils_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
			"%{Library.SPIRV_Tools_Debug}"
        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {
            "vendor/assimp/bin/Release/assimp-vc141-mt.lib",
			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.SPIRV_Tools}"
        }
        
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

		links
        {
            "vendor/assimp/bin/Release/assimp-vc141-mt.lib",
			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
        }
