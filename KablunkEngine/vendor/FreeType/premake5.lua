project "FreeType"
	kind "StaticLib"
	language "C"

	targetdir ("bin/" .. outputdir .. "/%{prj.name}")
	objdir ("bin-int/" .. outputdir .. "/%{prj.name}")

	files
	{
        "include/**.h", 
        "src/**.h",
        
        "src/base/ftsystem.c",
        "src/base/ftinit.c",
        "src/base/ftdebug.c",
        "src/base/ftbase.c",

        "src/base/ftbbox.c",     -- recommended, see <ftbbox.h>
        "src/base/ftglyph.c",     -- recommended, see <ftglyph.h>

        "src/base/ftbdf.c",     -- optional, see <ftbdf.h>
        "src/base/ftbitmap.c",     -- optional, see <ftbitmap.h>
        "src/base/ftcid.c",     -- optional, see <ftcid.h>
        "src/base/ftfstype.c",     -- optional
        "src/base/ftgasp.c",     -- optional, see <ftgasp.h>
        "src/base/ftgxval.c",    -- optional, see <ftgxval.h>
        "src/base/ftmm.c",     -- optional, see <ftmm.h>
        "src/base/ftotval.c",     -- optional, see <ftotval.h>
        "src/base/ftpatent.c",     -- optional
        "src/base/ftpfr.c",     -- optional, see <ftpfr.h>
        "src/base/ftstroke.c",     -- optional, see <ftstroke.h>
        "src/base/ftsynth.c",     -- optional, see <ftsynth.h>
        "src/base/fttype1.c",     -- optional, see <t1tables.h>
        "src/type1/type1.c",       -- Type 1 font driver
        "src/type42/type42.c",     -- Type 42 font driver
        "src/base/ftwinfnt.c",     -- optional, see <ftwinfnt.h>

        "src/truetype/truetype.c",
        "src/winfonts/winfnt.c",

        "src/psnames/psnames.c",
        "src/autofit/autofit.c",
        "src/psaux/psaux.c",
        "src/pshinter/pshinter.c",

        "src/sfnt/sfnt.c",

        "src/raster/raster.c",     -- monochrome rasterizer
        "src/sdf/sdf.c",           -- Signed Distance Field driver
        "src/smooth/smooth.c"     -- anti-aliasing rasterizer
    }

    includedirs
    {
        "include"
    }

    defines
    {
        "FT2_BUILD_LIBRARY"
    }

	filter "system:windows"
		systemversion "latest"
		staticruntime "off"

	filter "system:linux"
		pic "on"
		systemversion "latest"
		staticruntime "off"

	filter "configurations:Debug"
		runtime "Debug"
		symbols "on"

	filter "configurations:Release"
		runtime "Release"
		optimize "on"