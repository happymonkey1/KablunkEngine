project "KablunkEditor"
kind "ConsoleApp"
language "C++"
cppdialect "C++20"
staticruntime "off"
conformancemode "off"
editandcontinue "Off"

targetdir("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
objdir("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

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
    "%{IncludeDir.ImGuizmo}",
    "%{IncludeDir.assimp}",
    "%{IncludeDir.freetype}",
    "%{IncludeDir.mono}",
    "%{IncludeDir.Vulkan}",
    "%{IncludeDir.boost}",
    "%{IncludeDir.miniaudio}",
    "%{IncludeDir.optick}",
    "%{IncludeDir.tl_expected}",
    "%{IncludeDir.robin_hood}",
    -- included bc of ICE in MSVC 17.7
    "%{IncludeDir.fmt}",
    "%{IncludeDir.tracy}",
    "%{IncludeDir.GameNetworkingSockets}",
    "%{IncludeDir.msdf_atlas_gen}",
    "%{IncludeDir.msdfgen}",
    "%{IncludeDir.msgpack}",
}

links
{
    "KablunkEngine",
    "freetype",
    "%{Library.mono}",
    "GLFW",
    "MSGPACK_NO_BOOST",
}

defines
{
    "KB_BUILD_DLL",
    "GLFW_DLL",
    "GLM_FORCE_DEFAULT_ALIGNED_GENTYPES",
    "GLM_FORCE_INTRINSICS",
    "STEAMNETWORKINGSOCKETS_STATIC_LINK",
    "MSDFGEN_PUBLIC=__declspec(dllimport)",
    "MSDF_ATLAS_PUBLIC=__declspec(dllimport)",
    "GLM_FORCE_DEPTH_ZERO_TO_ONE",
    "MSGPACK_NO_BOOST",
}

postbuildcommands
{
    -- copy glfw dll to bin
    '{COPY} "%{IncludeDir.GLFW}/../bin/%{cfg.buildcfg}-%{cfg.system}-%{cfg.architecture}/GLFW/glfw.dll" "%{cfg.targetdir}"'
}

filter "system:windows"
systemversion "latest"

filter "configurations:Debug"
defines "KB_DEBUG"
runtime "Debug"
symbols "on"
links
{
    "%{Library.assimp}",
}



postbuildcommands
{
    '{COPY} "../KablunkEngine/vendor/assimp/bin/Debug/assimp-vc143-mtd.dll" "%{cfg.targetdir}"',
    '{COPY} "../KablunkEngine/vendor/mono/bin/Debug/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
    '{COPY} "%{LibraryDir.VulkanSDK_Debug_Bin}/shaderc_sharedd.dll" "%{cfg.targetdir}"'
    -- '{COPY} "../bin/Release-windows-x86_64/Sandbox/Sandbox.dll" "%{cfg.targetdir}"'
}

filter "configurations:Release"
defines "KB_RELEASE"
runtime "Release"
optimize "on"

links
{
    "%{Library.assimp_release}",
}

postbuildcommands
{
    '{COPY} "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
    '{COPY} "../KablunkEngine/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
    '{COPY} "%{LibraryDir.VulkanSDK_Bin}/shaderc_shared.dll" "%{cfg.targetdir}"'
    -- '{COPY} "../bin/Release-windows-x86_64/Sandbox/Sandbox.dll" "%{cfg.targetdir}"'
}

filter "configurations:Distribution"
defines "KB_DISTRIBUTION"
runtime "Release"
optimize "on"
symbols "off"

links
{
    "%{Library.assimp_release}",
}

postbuildcommands
{
    '{COPY} "../KablunkEngine/vendor/assimp/bin/Release/assimp-vc143-mt.dll" "%{cfg.targetdir}"',
    '{COPY} "../KablunkEngine/vendor/mono/bin/Release/mono-2.0-sgen.dll" "%{cfg.targetdir}"',
    '{COPY} "%{LibraryDir.VulkanSDK_Bin}/shaderc_shared.dll" "%{cfg.targetdir}"'
    -- '{COPY} "../bin/Release-windows-x86_64/Sandbox/Sandbox.dll" "%{cfg.targetdir}"'
}
