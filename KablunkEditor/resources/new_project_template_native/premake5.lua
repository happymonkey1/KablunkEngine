ProjectName = "$PROJECT_NAME$"
KablunkRootDirectory = os.getenv("KABLUNK_DIR")
include "Dependencies.lua"



workspace "%{ProjectName}"
    architecture "x64"
	targetdir "build"
    startproject "%{ProjectName}"

    configurations
    {
        "Debug",
        "Release",
        "Distribution"
    }

    flags
    {
        "MultiProcessorCompile"
    }
project "$PROJECT_NAME$"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	targetname "%{ProjectName}"

	targetdir ("%{wks.location}/bin/")
	objdir ("%{wks.location}/bin-int/%{prj.name}")


	files
	{
		"include/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"include",
		"src",
		
		"%{KablunkRootDirectory}/KablunkEngine/include/",

		"%{IncludeDir.spdlog}",
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
		"%{IncludeDir.boost}"
	}

	links
	{
		"%{Library.Kablunk}",
		"%{Library.GLFW}",
		"%{Library.ImGui}",
		"%{Library.Box2d}",
		"%{Library.Glad}",
		"%{Library.yaml_cpp}"
	}

	defines
	{
		"GLFW_DLL"
	}

	filter "system:windows"
		systemversion "latest"
	
		filter "configurations:Debug"
		defines "KB_DEBUG"
		runtime "Debug"
		symbols "on"

		links
        {

        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {

        }
        
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

		links
        {
			
        }