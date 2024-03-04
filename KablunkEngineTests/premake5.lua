project "KablunkEngineTests"
    kind "ConsoleApp"
    language "C++"
    cppdialect "C++20"
	staticruntime "off"
	conformancemode "off"

    targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
    objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

    files {
        "include/**test.h",
        "src/**test.cpp",
        "src/Kablunk/entrypoint.cpp",
    }

    includedirs {
        "%{wks.location}/KablunkEngine/include/",
        "%{IncludeDir.Catch2}",
        "%{IncludeDir.robin_hood}",
        "%{IncludeDir.LuaJIT}",
        "%{IncludeDir.tl_expected}",
        "%{IncludeDir.spdlog}",
        "%{IncludeDir.mono}",
        "%{IncludeDir.fmt}",
    }

    links {
        "KablunkEngine",
        "Catch2",
        "%{Library.LuaJIT}"
    }

    postbuildcommands {
		'{COPY} "%{wks.location}KablunkEngine/vendor/LuaJIT/src/lua51.dll" "%{cfg.targetdir}"',
        '{COPY} "%{wks.location}KablunkEngineTests/resources/lua" "%{cfg.targetdir}/lua"',
	}

    filter "configurations:Debug"
        defines {
            "KB_DEBUG"
        }
        
        runtime "Debug"
        symbols "on"

    filter "configurations:Release"
        flags { "LinkTimeOptimization" }
        defines {
            "KB_RELEASE"
        }

        runtime "Release"
        optimize "on"
        symbols "on"

    filter "configurations:Distribution"
        flags { "LinkTimeOptimization" }
        defines {
            "KB_DISTRIBUTION"
        }

        runtime "Release"
        optimize "on"
        symbols "off"