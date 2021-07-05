include "./vendor/premake/premake_customization/solution_items.lua"
include "Dependencies.lua"

workspace "KablunkEngine"
    architecture "x86_64"
    startproject "KablunkEditor"

    configurations
    {
        "Debug",
        "Release",
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
    include "KablunkEngine/vendor/GLFW"
    include "KablunkEngine/vendor/Glad"
    include "KablunkEngine/vendor/ImGui"
group ""

include "KablunkEngine"
include "KablunkEditor"
include "Sandbox"