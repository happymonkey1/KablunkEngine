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