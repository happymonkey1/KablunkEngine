ProjectName = "$PROJECT_NAME$"
KablunkRootDirectory = os.getenv("KABLUNK_DIR")

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
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"%{KablunkRootDirectory}/KablunkEngine/include/**.h",
		"%{KablunkRootDirectory}/KablunkEngine/include/**.hpp"
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