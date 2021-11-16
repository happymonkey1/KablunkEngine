project_name = "$PROJECT_NAME$"
kablunk_root_dir = os.getenv("KABLUNK_DIR")
kablunk_root_dir = kablunk_root_dir:gsub("\\", "/")

include(dependencies_path)

workspace "%{project_name}"
    architecture "x64"
	targetdir "build"
    startproject "%{project_name}"

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
	location "%{kablunk_root_dir}/Kablunk-ScriptCore"
	kind "SharedLib"
	language "C#"

	targetdir ("%{kablunk_root_dir}/KablunkEditor/resources/scripts")
	objdir ("%{kablunk_root_dir}/KablunkEditor/resources/scripts/intermediates")

	files
	{
		"%{kablunk_root_dir}/Kablunk-ScriptCore/Source/**.cs"
	}
group ""


project "$PROJECT_NAME$"
	location "Assets/Scripts"
	kind "SharedLib"
	language "C#"

	targetname "%{project_name}"
	targetdir ("%{prj.location}/binaries")
	objdir ("%{kablunk_root_dir}/KablunkEditor/resources/scripts/intermediates")

	files
	{
		"assets/scripts/source/**.cs"
	}

	links
	{
		"Kablunk-ScriptCore"
	}