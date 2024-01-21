project "KablunkEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++20"
	staticruntime "off"
	conformancemode "off"
	vectorextensions "AVX2"
	editandcontinue "Off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	pchheader "kablunkpch.h"
	pchsource "src/kablunkpch.cpp"

	files {
		"include/**.h",
		"include/**.hpp",
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
		"vendor/cr/cr.h",

		"vendor/expected/include/**.hpp",

		"vendor/tracy/public/TracyClient.cpp",
		"vendor/tracy/public/tracy/Tracy.hpp",
	}

	defines {
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"NOMINMAX",
		"KB_BUILD_DLL",
		"GLFW_DLL",
		"GLM_FORCE_DEFAULT_ALIGNED_GENTYPES",
		"GLM_FORCE_INTRINSICS",
	}

	includedirs {
		"include",
		"src",
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
		"%{IncludeDir.VulkanSDK_LocalInclude}",
		"%{IncludeDir.boost}",
		"%{IncludeDir.miniaudio}",
		"%{IncludeDir.optick}",
		"%{IncludeDir.tl_expected}",
		"%{IncludeDir.robin_hood}",
		"%{IncludeDir.protobuf}",
		"%{IncludeDir.absl}",
		-- external fmt lib because of MSVC 17.7 bug
		"%{IncludeDir.fmt}",
		"%{IncludeDir.tracy}",
		"%{IncludeDir.GameNetworkingSockets}",
	}

	links {
		--"GLFW",
		"Glad",
		"ImGui",
		"yaml-cpp",
		"spdlog",
		"FreeType",
		"Box2d",
		"opengl32.lib",
		"optick",
		"absl",
		"protobuf",
		"protoc",
		-- external fmt lib because of MSVC 17.7 bug
		"fmt",
		"GameNetworkingSockets",

		"%{Library.Vulkan}",
		-- "%{Library.VulkanUtils}",

		
	}

	postbuildcommands {
		'{COPY} "%{IncludeDir.GLFW}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/GLFW/glfw.dll" "%{cfg.targetdir}"'
	}
	

	filter "files:vendor/ImGuizmo/**.cpp"
		flags { "NoPCH" }

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines {
			"KB_DEBUG",
			-- "TRACY_ENABLE",
		}
		runtime "Debug"
		symbols "on"

		links
        {
            "%{Library.assimp}",
			"%{Library.ShaderC_Debug}",
			"%{Library.ShaderC_Utils_Debug}",
			"%{Library.SPIRV_Cross_Debug}",
			"%{Library.SPIRV_Cross_GLSL_Debug}",
			"%{Library.SPIRV_Tools_Debug}",
			"%{Library.mono}",
        }
	
	filter "configurations:Release"
		flags { "LinkTimeOptimization" }
		defines {
			"KB_RELEASE",
			"TRACY_ENABLE",
			"TRACY_ON_DEMAND", -- incurs a slight performance overhead
		}
		runtime "Release"
		optimize "on"

		links {
            "%{Library.assimp_release}",
			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.SPIRV_Tools}",
			"%{Library.mono_release}",
        }
        
	
	filter "configurations:Distribution"
		flags { "LinkTimeOptimization" }
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"
		symbols "off"

		links {
            "%{Library.assimp_release}",
			"%{Library.ShaderC_Release}",
			"%{Library.ShaderC_Utils_Release}",
			"%{Library.SPIRV_Cross_Release}",
			"%{Library.SPIRV_Cross_GLSL_Release}",
			"%{Library.mono_release}"
        }
