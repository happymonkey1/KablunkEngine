project_name = "$PROJECT_NAME$"
kablunk_root_dir = os.getenv("KABLUNK_DIR")
kablunk_root_dir = kablunk_root_dir:gsub("\\", "/")
dependencies_path = kablunk_root_dir .. "/dependencies.lua"

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
	staticruntime "off"

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"src/**.cpp",
		"include/**.h"
		
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS"
	}

	includedirs
	{
		"include",
		"$KABLUNK_DIR$/KablunkEngine/src",
		"$KABLUNK_DIR$/%{IncludeDir.glm}",
		"$KABLUNK_DIR$/%{IncludeDir.stb_image}",
		"$KABLUNK_DIR$/%{IncludeDir.entt}",
		"$KABLUNK_DIR$/%{IncludeDir.yaml_cpp}",
		"$KABLUNK_DIR$/%{IncludeDir.spdlog}",
		"$KABLUNK_DIR$/%{IncludeDir.stduuid}",
		"$KABLUNK_DIR$/%{IncludeDir.gsl}",
		"$KABLUNK_DIR$/%{IncludeDir.ImGuizmo}",
		"$KABLUNK_DIR$/%{IncludeDir.assimp}",
		"$KABLUNK_DIR$/%{IncludeDir.FreeType}",
		"$KABLUNK_DIR$/%{IncludeDir.Box2d}"
	}

	links
	{
		"%{kablunk_root_dir}/bin/" .. outputdir .. "/KablunkEngine/KablunkEngine.lib"
	}

	filter "system:windows"
		systemversion "latest"
		linkoptions { "/FORCE:UNRESOLVED" }

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
