project_name = "$PROJECT_NAME$"
kablunk_root_dir = os.getenv("KABLUNK_DIR")
kablunk_root_dir = kablunk_root_dir:gsub("\\", "/")
dependencies_path = kablunk_root_dir .. "\\dependencies.lua"
dependencies_path = dependencies_path:gsub("\\", "/")

include(dependencies_path)

workspace "%{project_name}"
    architecture "x86_64"
    startproject "%{project_name}"

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

	outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

project "$PROJECT_NAME$"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cpp",
		"include/**.h"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"NOMINMAX"
	}

	includedirs
	{
		"src",
		"include",
		"KablunkEngine/engine",
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
		"%{IncludeDir.Box2d}"
	}

	links
	{
		"%{kablunk_root_dir}/bin/" .. outputdir .. "/KablunkEngine/KablunkEngine.lib"
	}

	filter "system:windows"
		systemversion "latest"
		--linkoptions { "/FORCE:UNRESOLVED" }

	filter "configurations:Debug"
		defines "KB_DEBUG"
		runtime "Debug"
		symbols "on"

		links
        {
            
        }

		postbuildcommands
        {
            '{COPY} "%{cfg.targetdir}/%{file.basename}.dll" "assets/bin"'
        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {
            
        }

		postbuildcommands
        {
            '{COPY} "%{cfg.targetdir}/%{file.basename}.dll" "assets/bin"'
        }
        
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

		links
        {
            
        }

		postbuildcommands
        {
            '{COPY} "../KablunkEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"'
        }
