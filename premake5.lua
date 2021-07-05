include "Dependencies.lua"

workspace "KablunkEngine"
    architecture "x86_64"
    startproject "KablunkEditor"

    configurations
    {
        "Debug"
        "Release"
        "Distribution"
    }

    solution_items
    {
        ".editorconfig"
    }

    flags
    {
        "MultiProcessorCompile"
    }

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
    include "vendor/premake"
    include "KablunkEngine2020/vendor/GLFW"
    include "KablunkEngine2020/vendor/Glad"
    include "KablunkEngine2020/vendor/imgui"
group ""

include "KablunkEngine"
include "Sandbox"
include "KablunkEditor"