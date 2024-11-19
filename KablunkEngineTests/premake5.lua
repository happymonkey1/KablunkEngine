project "KablunkEngineTests"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
staticruntime "off"
conformancemode "off"
editandcontinue "Off"

targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

files {
    "include/**test.h",
    "src/**test.cpp",
    "src/Kablunk/entrypoint.cpp",
}

includedirs {
    "%{wks.location}/KablunkEngine/include/",
    "%{IncludeDir.GLFW}",
	"%{IncludeDir.Glad}",
    "%{IncludeDir.Catch2}",
    "%{IncludeDir.robin_hood}",
    "%{IncludeDir.LuaJIT}",
    "%{IncludeDir.tl_expected}",
    "%{IncludeDir.spdlog}",
    "%{IncludeDir.mono}",
    "%{IncludeDir.fmt}",
    "%{IncludeDir.GameNetworkingSockets}",
    "%{IncludeDir.stduuid}",
    "%{IncludeDir.msgpack}",
    "%{IncludeDir.boost}",
    "%{IncludeDir.glm}",
    "%{IncludeDir.rapidjson}",
}

links {
    "KablunkEngine",
    "Catch2",
    "%{Library.LuaJIT}",
    "GLFW"
}

postbuildcommands {
    '{COPY} "%{wks.location}KablunkEngine/vendor/LuaJIT/src/lua51.dll" "%{cfg.targetdir}"',
    '{COPY} "%{IncludeDir.GLFW}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/GLFW/glfw.dll" "%{cfg.targetdir}"',
    '{COPY} "%{wks.location}KablunkEngineTests/resources/lua" "%{cfg.targetdir}/lua"',
    '{COPY} "%{wks.location}KablunkEngine/vendor/mono/bin/%{cfg.buildcfg}/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
}

defines {
    "_DISABLE_VECTOR_ANNOTATION",
    "_DISABLE_STRING_ANNOTATION",
    "GLFW_DLL",
    "KB_PACKED_MATH_TYPES"
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
