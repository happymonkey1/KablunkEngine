project "KablunkEngine"
	kind "StaticLib"
	language "C++"
	cppdialect "C++17"
	staticruntime "on"

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
		"vendor/ImGuizmo/ImGuizmo.cpp"
	}

	defines
	{
		"_CRT_SECURE_NO_WARNINGS",
		"GLFW_INCLUDE_NONE",
		"NOMINMAX",
		"KB_BUILD_DLL",
		"KB_NATIVE_SCRIPTING"
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
		"%{IncludeDir.Box2d}"
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
		"opengl32.lib"
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
            "vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {
            "vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }
        
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

		links
        {
            "vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }
