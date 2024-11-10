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
    ".editorconfig",
    ".clang-tidy"
}

flags
{
    "MultiProcessorCompile"
}

outputdir = "%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}"

group "Dependencies"
include "vendor/premake"
include "KablunkEngine/vendor/abseil-cpp" -- Needed for GameNetworkingSockets
include "KablunkEngine/vendor/GLFW"
include "KablunkEngine/vendor/Glad"
include "KablunkEngine/vendor/imgui"
include "KablunkEngine/vendor/yaml-cpp"
include "KablunkEngine/vendor/spdlog"
include "KablunkEngine/vendor/box2d"
include "KablunkEngine/vendor/fmt" -- external fmt lib because of MSVC 17.7 bug
include "KablunkEngine/vendor/GameNetworkingSockets"
include "KablunkEngine/vendor/Catch2"
include "KablunkEngine/vendor/msdf-atlas-gen"
include "KablunkEngine/vendor/tinyxml2"
group ""

group "Tools"
include "KablunkEditor"
group ""

group "Core"
include "KablunkEngine"
include "Kablunk-ScriptCore"
include "KablunkEngineTests"
group ""
