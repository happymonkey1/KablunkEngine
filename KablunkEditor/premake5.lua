project "KablunkEditor"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++17"
    staticruntime "on"

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
		"%{wks.location}/KablunkEngine/include",
		"%{wks.location}/KablunkEngine/vendor",
		"%{wks.location}/Sandbox/src",
		"%{IncludeDir.glm}",
		"%{IncludeDir.entt}",
		"%{IncludeDir.ImGui}",
		"%{IncludeDir.yaml_cpp}",
        "%{IncludeDir.stduuid}",
        "%{IncludeDir.gsl}",
		"%{IncludeDir.ImGuizmo}",
        "%{IncludeDir.assimp}",
        "%{IncludeDir.FreeType}",
        "%{IncludeDir.RCCPP}",
        "%{IncludeDir.mono}"
    }

    links
    {
        "KablunkEngine",
        "Sandbox",
        "FreeType",
        "%{Library.mono}"
    }

    defines
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
            "../KablunkEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.lib"
        }

        postbuildcommands
        {
            '{COPY} "../KablunkEngine/vendor/assimp/bin/Debug/assimp-vc141-mtd.dll" "%{cfg.targetdir}"',
            '{COPY} "../KablunkEngine/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

        links
        {
            "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }
        
        postbuildcommands
        {
            '{COPY} "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc141-mt.dll" "%{cfg.targetdir}"',
            '{COPY} "../KablunkEngine/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"'
        }
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

        links
        {
            "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc141-mt.lib"
        }
        
        postbuildcommands
        {
            '{COPY} "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc141-mt.dll" "%{cfg.targetdir}"'
        }
