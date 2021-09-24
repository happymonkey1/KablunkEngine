project_name = "$PROJECT_NAME$";

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

project "%{project_name}"
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
		"include"
	}

	links
	{

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
