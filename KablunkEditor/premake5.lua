project "KablunkEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files
    {
        "src/**.h",
        "src/**.cpp"
    }

    includedirs
    {
        "%{wks.location}/KablunkEngine2020/vendor/spdlog/include",
		"%{wks.location}/KablunkEngine2020/src",
		"%{wks.location}/KablunkEngine2020/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
    }

    links
    {
        "KablunkEngine"
    }

    filer "system:windows"
		systemversion "latest"

		defines 
		{
		}

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
		symbols "on"

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