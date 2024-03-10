#include "kablunkpch.h"
#include "Kablunk/Renderer/Font/font_atlas.h"

namespace kb::render
{ // start namespace kb::render


auto generate_atlas(const std::filesystem::path& p_filepath) noexcept -> bool
{
    /*
    bool success = false;
    // Initialize instance of FreeType library
    if (msdfgen::FreetypeHandle* ft = msdfgen::initializeFreetype())
    {
        // Load font file
        if (msdfgen::FontHandle* font = msdfgen::loadFont(ft, p_filepath.string().c_str()))
        {
            // Storage for glyph geometry and their coordinates in the atlas
            std::vector<msdf_atlas::GlyphGeometry> glyphs;
            // FontGeometry is a helper class that loads a set of glyphs from a single font.
            // It can also be used to get additional font metrics, kerning information, etc.
            msdf_atlas::FontGeometry font_geometry{ &glyphs };
            // Load a set of character glyphs:
            // The second argument can be ignored unless you mix different font sizes in one atlas.
            // In the last argument, you can specify a charset other than ASCII.
            // To load specific glyph indices, use loadGlyphs instead.
            font_geometry.loadCharset(font, 1.0, msdf_atlas::Charset::ASCII);
            // Apply MSDF edge coloring. See edge-coloring.h for other coloring strategies.
            const double max_corner_angle = 3.0;
            for (msdf_atlas::GlyphGeometry& glyph : glyphs)
                glyph.edgeColoring(&msdfgen::edgeColoringInkTrap, max_corner_angle, 0);
            // TightAtlasPacker class computes the layout of the atlas.
            msdf_atlas::TightAtlasPacker packer;
            // Set atlas parameters:
            // setDimensions or setDimensionsConstraint to find the best value
            packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::SQUARE);
            // setScale for a fixed size or setMinimumScale to use the largest that fits
            packer.setMinimumScale(24.0);
            // setPixelRange or setUnitRange
            packer.setPixelRange(2.0);
            packer.setMiterLimit(1.0);
            // Compute atlas layout - pack glyphs
            packer.pack(glyphs.data(), glyphs.size());
            // Get final atlas dimensions
            int width = 0, height = 0;
            packer.getDimensions(width, height);
            // The ImmediateAtlasGenerator class facilitates the generation of the atlas bitmap.
            msdf_atlas::ImmediateAtlasGenerator<
                float, // pixel type of buffer for individual glyphs depends on generator function
                3, // number of atlas color channels
                &msdf_atlas::msdfGenerator, // function to generate bitmaps for individual glyphs
                msdf_atlas::BitmapAtlasStorage<msdf_atlas::byte, 3> // class that stores the atlas bitmap
                // For example, a custom atlas storage class that stores it in VRAM can be used.
            > generator{ width, height };
            // GeneratorAttributes can be modified to change the generator's default settings.
            msdf_atlas::GeneratorAttributes attributes{};
            generator.setAttributes(attributes);
            generator.setThreadCount(4);
            // Generate atlas bitmap
            generator.generate(glyphs.data(), glyphs.size());
            // The atlas bitmap can now be retrieved via atlasStorage as a BitmapConstRef.
            // The glyphs array (or fontGeometry) contains positioning data for typesetting text.
            //success = myProject::submitAtlasBitmapAndLayout(generator.atlasStorage(), glyphs);
            // Cleanup
            msdfgen::destroyFont(font);
        }
        msdfgen::deinitializeFreetype(ft);
    }
    return success;*/
    KB_CORE_ASSERT(false, "not implemented");
    return false;
}


} // end namespace kb::render
