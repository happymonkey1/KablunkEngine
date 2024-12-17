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

--- Check if a file or directory exists in this path
function exists(file)
    local ok, err, code = os.rename(file, file)
    if not ok then
       if code == 13 then
          -- Permission denied, but it exists
          return true
       end
    end
    return ok, err
 end
 
 --- Check if a directory exists in this path
 function isdir(path)
    -- "/" works on both Unix and Windows
    return exists(path.."/")
 end


function include_sandbox_if_exists(path)
    if not isdir(path) then
        return
    end

    include(path)
end

group "Sandbox"
include_sandbox_if_exists("./kablunk-sandbox/kb-pve-moba")
group ""
