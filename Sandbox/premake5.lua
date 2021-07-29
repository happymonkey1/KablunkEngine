project "Sandbox"
	kind "StaticLib"
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
		"src",
		"%{wks.location}/KablunkEngine/vendor/spdlog/include",
		"%{wks.location}/KablunkEngine/src",
		"%{wks.location}/KablunkEngine/vendor",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGui}",
        "%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.stduuid}",
        "%{IncludeDir.gsl}"
	}

	links
	{
		"KablunkEngine"
	}

	filter "system:windows"
		systemversion "latest"

	filter "configurations:Debug"
		defines "KB_DEBUG"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"