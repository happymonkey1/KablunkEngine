ProjectName = "$PROJECT_NAME$"
KablunkRootDirectory = os.getenv("KABLUNK_DIR")

workspace "%{ProjectName}"
    architecture "x64"
	targetdir "build"
    startproject "%{ProjectName}"

    configurations
    {
        "Debug",
        "Release",
        "Distribution"
    }

    flags
    {
        "MultiProcessorCompile"
    }

group "Kablunk"
project "Kablunk-ScriptCore"
	location "%{KablunkRootDirectory}/Kablunk-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("%{KablunkRootDirectory}/KablunkEditor/resources/scripts")
	objdir ("%{KablunkRootDirectory}/KablunkEditor/resources/scripts/intermediates")

	files
	{
		"%{KablunkRootDirectory}/Kablunk-ScriptCore/Source/**.cs"
	}
group ""


project "$PROJECT_NAME$"
	location "Assets/Scripts"
	kind "SharedLib"
	language "C#"

	targetname "%{ProjectName}"
	targetdir ("%{prj.location}/binaries")
	objdir ("%{KablunkRootDirectory}/KablunkEditor/resources/scripts/intermediates")

	files
	{
		"assets/scripts/source/**.cs"
	}

	links
	{
		"Kablunk-ScriptCore"
	}

project "$PROJECT_NAME$-Native"
	kind "SharedLib"
	language "C++"
	cppdialect "C++17"

	targetname "%{ProjectName}"

	targetdir ("%{wks.location}/bin/%{prj.name}")
	objdir ("%{wks.location}/bin-int/%{prj.name}")


	files
	{
		"src/**.h",
		"src/**.cpp"
	}

	includedirs
	{
		"KablunkEngine/engine/",
		"KablunkEngine/vendor/",
		"KablunkEngine/vendor/assimp/include",
		"KablunkEgnine/vendor/box2d/include",
		"KablunkEngine/vendor/cr",
		"KablunkEngine/vendor/entt/include",
		"KablunkEngine/vendor/FreeType/include",
		"KablunkEngine/vendor/Glad/include",
		"KablunkEngine/vendor/GLFW/include",
		"KablunkEngine/vendor/glm/",
		"KablunkEngine/vendor/imgui",
		"KablunkEngine/vendor/ImGuizmo",
		"KablunkEngine/vendor/mono/include",
		"KablunkEngine/vendor/spdlog/include",
		"KablunkEngine/vendor/stb_image",
		"KablunkEngine/vendor/yaml-cpp/include"
	}

	links
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

        }
	
	filter "configurations:Release"
		defines "KB_RELEASE"
		runtime "Release"
		optimize "on"

		links
        {

        }
        
	
	filter "configurations:Distribution"
		defines "KB_DISTRIBUTION"
		runtime "Release"
		optimize "on"

		links
        {
			
        }