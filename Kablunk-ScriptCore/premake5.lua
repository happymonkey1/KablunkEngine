project "Kablunk-ScriptCore"
	kind "SharedLib"
	language "C#"
	csversion ("4.8")

	targetdir ("%{wks.location}/bin/" .. outputdir .. "/%{prj.name}")
	objdir ("%{wks.location}/bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
		"source/**.cs"
	}