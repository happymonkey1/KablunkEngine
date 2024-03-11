#include "kablunkpch.h"
#include "Kablunk/Renderer/Font/font.h"

#include "Kablunk/Core/owning_buffer.h"

namespace kb::render
{ // start namespace kb::render

namespace
{
// from ImGui
constexpr size_t k_charset_range = 9ull;
constexpr u32 k_charset_ranges[k_charset_range]
{
    0x0020, 0x00FF, // Basic Latin + Latin Supplement
    0x0400, 0x052F, // Cyrillic + Cyrillic Supplement
    0x2DE0, 0x2DFF, // Cyrillic Extended-A
    0xA640, 0xA69F, // Cyrillic Extended-B
    0,
};

std::filesystem::path s_cache_directory = "resources/cache/font_atlases";
}

constexpr u64 k_lcg_multiplier = 6364136223846793005ull;
constexpr u64 k_lcg_increment = 1442695040888963407ull;
// #TODO should check if this exceeds thread count of system
constexpr u64 k_default_thread_count = 8;

struct font_load_descriptor
{
    owning_buffer m_buffer{};
    msdf_atlas::GlyphIdentifierType m_glyph_identifier_type{};
    std::string_view m_charset_filename{};
    f64 m_font_scale = 1.;
    std::string_view m_font_name{};
};

struct font_config
{
    using edge_coloring_func_t = void (*)(msdfgen::Shape&, f64, u64);

    msdf_atlas::ImageType m_image_type{};
    msdf_atlas::ImageFormat m_image_format{};
    msdf_atlas::YDirection m_y_direction{};
    i32 m_width = 0;
    i32 m_height = 0;
    f64 m_em_size = 0.;
    f64 m_pixel_range = 0.;
    f64 m_angle_threshold = 0.;
    f64 m_miter_limit = 0.;
    edge_coloring_func_t m_edge_coloring_func = nullptr;
    bool m_expensive_coloring = true;
    u64 m_coloring_seed = 0;
    msdf_atlas::GeneratorAttributes m_generator_attributes{};
};

namespace details
{ // start namespace ::details

struct atlas_header
{
    u32 m_width = 0;
    u32 m_height = 0;
};

static auto try_read_cached_font_atlas(
    std::string_view p_name,
    const f32 p_font_size,
    atlas_header& p_atlas_header,
    void*& p_pixel_data,
    owning_buffer& p_storage_buffer
) noexcept -> bool
{
    const auto kb_font_filename = fmt::format("{}-{}.kbf", p_name, p_font_size);
    const auto filepath = s_cache_directory / kb_font_filename;

    if (std::filesystem::exists(filepath))
    {
        p_storage_buffer = FileSystem::read_bytes(filepath);
        p_atlas_header = *p_storage_buffer.As<atlas_header>();
        p_pixel_data = static_cast<u8*>(p_storage_buffer.get()) + sizeof(atlas_header);
        return true;
    }

    return false;
}

static auto cache_font_atlas(
    std::string_view p_name,
    f32 p_font_size,
    atlas_header p_atlas_header,
    const void* p_pixel_data
) noexcept -> void
{
    if (!std::filesystem::exists(s_cache_directory))
        std::filesystem::create_directories(s_cache_directory);

    const auto kb_font_filename = fmt::format("{}-{}.kbf", p_name, p_font_size);
    const auto filepath = s_cache_directory / kb_font_filename;

    std::ofstream file{ filepath, std::ios::binary | std::ios::trunc };
    KB_CORE_ASSERT(file, "[font]: Failed to write font atlas to cache file {}", filepath.string().c_str());

    file.write(reinterpret_cast<char*>(&p_atlas_header), sizeof(atlas_header));
    // #TODO should probably expose channel count and color depth
    constexpr size_t k_channels = 4ull;
    constexpr size_t k_color_depth = sizeof(f32);
    const size_t pixel_buf_size = static_cast<size_t>(p_atlas_header.m_width) * static_cast<size_t>(p_atlas_header.m_height) * k_color_depth * k_channels;
    file.write(static_cast<const char*>(p_pixel_data), static_cast<i64>(pixel_buf_size));
}

template <typename T, typename S, int N, msdf_atlas::GeneratorFunction<S, N> GenFuncT>
static auto create_and_cache_atlas(
    std::string_view p_name,
    f32 p_font_size,
    const std::vector<msdf_atlas::GlyphGeometry>& p_glyphs,
    const msdf_atlas::FontGeometry& p_font_geometry,
    const font_config& p_font_config
) noexcept -> ref<Texture2D>
{
    msdf_atlas::ImmediateAtlasGenerator<S, N, GenFuncT, msdf_atlas::BitmapAtlasStorage<T, N>> generator{ p_font_config.m_width, p_font_config.m_height };
    generator.setAttributes(p_font_config.m_generator_attributes);
    generator.setThreadCount(k_default_thread_count);
    generator.generate(p_glyphs.data(), static_cast<i32>(p_glyphs.size()));

    auto bitmap = static_cast<msdfgen::BitmapConstRef<T, N>>(generator.atlasStorage());

    const details::atlas_header atlas_header{
        .m_width = static_cast<u32>(bitmap.width),
        .m_height = static_cast<u32>(bitmap.height),
    };
    cache_font_atlas(p_name, p_font_size, atlas_header, bitmap.pixels);

    return Texture2D::Create(ImageFormat::RGBA32F, atlas_header.m_width, atlas_header.m_height, bitmap.pixels);
}

static auto create_atlas(
    const atlas_header p_atlas_header,
    const void* p_pixel_data
) noexcept -> ref<Texture2D>
{
    return Texture2D::Create(ImageFormat::RGBA32F, p_atlas_header.m_width, p_atlas_header.m_height, p_pixel_data);
}

} // end namespace ::details

font::font(const std::filesystem::path& p_path) noexcept
    : m_msdf_metrics{ new msdf_metrics{} }
{
    KB_CORE_ASSERT(std::filesystem::exists(p_path), "[font]: {} does not exist!", p_path.string().c_str());
    m_name = p_path.stem().string();

    KB_CORE_INFO("[font]: Trying to load font file '{}'", p_path);

    owning_buffer buffer = FileSystem::read_bytes(p_path);
    generate_atlas(std::move(buffer));
}

font::font(std::string p_name, owning_buffer&& p_buffer) noexcept
    : m_name{ std::move(p_name) }, m_msdf_metrics{ new msdf_metrics{} }
{
    generate_atlas(std::move(p_buffer));
}

auto font::generate_atlas(owning_buffer&& p_font_data_buffer) noexcept -> void
{
    const font_load_descriptor load_descriptor{
        .m_buffer = std::move(p_font_data_buffer),
        .m_glyph_identifier_type = msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT,
        .m_font_scale = 1.,
    };

    constexpr f64 k_default_angle_threshold = 3.0;
    constexpr f64 k_default_miter_limit = 3.0;
    font_config config{
        .m_image_type = msdf_atlas::ImageType::MTSDF,
        .m_image_format = msdf_atlas::ImageFormat::BINARY_FLOAT,
        .m_y_direction = msdf_atlas::YDirection::BOTTOM_UP,
        .m_em_size = 96.,
        .m_angle_threshold = k_default_angle_threshold,
        .m_miter_limit = k_default_miter_limit,
        .m_edge_coloring_func = msdfgen::edgeColoringInkTrap,
    };

    config.m_generator_attributes.config.overlapSupport = true;
    config.m_generator_attributes.scanlinePass = true;

    // #TODO: can we not initialize freetype for every font...
    const auto ft_handle = msdfgen::initializeFreetype();
    msdfgen::FontHandle* font_handle = msdfgen::loadFontData(
        ft_handle,
        load_descriptor.m_buffer.As<const msdfgen::byte>(),
        static_cast<i32>(load_descriptor.m_buffer.size())
    );
    KB_CORE_ASSERT(font_handle, "[font]: msdfgen failed to load font data from buffer!");

    // load charsets
    msdf_atlas::Charset charset{};
    for (size_t range = 0; range < k_charset_range; range += 2)
        for (uint32_t c = k_charset_ranges[range]; c <= k_charset_ranges[range + 1]; c++)
            charset.add(c);

    // load glyphs
    m_msdf_metrics->m_font_geometry = msdf_atlas::FontGeometry{ &m_msdf_metrics->m_glyphs };
    bool codepoints_available = false;
    i32 glyphs_loaded = -1;
    switch (load_descriptor.m_glyph_identifier_type)
    {
    case msdf_atlas::GlyphIdentifierType::GLYPH_INDEX:
        KB_CORE_INFO("[font]: Loading glyph indices for font");
        glyphs_loaded = m_msdf_metrics->m_font_geometry.loadGlyphset(font_handle, load_descriptor.m_font_scale, charset);
        break;
    case msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT:
        KB_CORE_INFO("[font]: Loading codepoints for font");
        glyphs_loaded = m_msdf_metrics->m_font_geometry.loadCharset(font_handle, load_descriptor.m_font_scale, charset);
        codepoints_available = glyphs_loaded > 0;
        break;
    }

    KB_CORE_ASSERT(glyphs_loaded > 0, "[font]: Failed to glyphs!");
    KB_CORE_INFO("[font]: Loaded {} glyphs out of {} charsets", glyphs_loaded, charset.size());

    if (glyphs_loaded < static_cast<i32>(charset.size()))
    {
        KB_CORE_WARN(
            "[font]: Could not load {} {}.",
            charset.size() - glyphs_loaded,
            load_descriptor.m_glyph_identifier_type == msdf_atlas::GlyphIdentifierType::UNICODE_CODEPOINT ? "codepoints" : "glyphs"
        );
    }

    if (!load_descriptor.m_font_name.empty())
        m_msdf_metrics->m_font_geometry.setName(load_descriptor.m_font_name.data());

    // clean up resources
    if (font_handle)
        msdfgen::destroyFont(font_handle);

    if (ft_handle)
        msdfgen::deinitializeFreetype(ft_handle);

    constexpr f64 fixed_width = -1.;
    constexpr f64 fixed_height = -1.;
    constexpr bool fixed_dimensions = fixed_width > 0 && fixed_height > 0;

    // config atlas packer
    {
        msdf_atlas::TightAtlasPacker atlas_packer{};
        if (fixed_dimensions)
            atlas_packer.setDimensions(fixed_width, fixed_height);
        else
            atlas_packer.setDimensionsConstraint(msdf_atlas::TightAtlasPacker::DimensionsConstraint::MULTIPLE_OF_FOUR_SQUARE);

        atlas_packer.setPadding(config.m_image_type == msdf_atlas::ImageType::MSDF || config.m_image_type == msdf_atlas::ImageType::MTSDF ? 0 : -1);

        if (config.m_em_size > 0.)
            atlas_packer.setScale(config.m_em_size);
        else
            atlas_packer.setMinimumScale(0.);

        constexpr f64 pixel_range = 2.0;
        atlas_packer.setPixelRange(pixel_range);

        atlas_packer.setMiterLimit(config.m_miter_limit);

        int remaining = atlas_packer.pack(m_msdf_metrics->m_glyphs.data(), static_cast<i32>(m_msdf_metrics->m_glyphs.size()));
        if (remaining > 0)
        {
            KB_CORE_ASSERT(false, "[font]: Could not fit {} out of {} glyphs in the atlas!", remaining, static_cast<i32>(m_msdf_metrics->m_glyphs.size()));
        }
        else if (remaining < 0)
        {
            KB_CORE_ASSERT(false, "[font]: Atlas packer failed!");
        }

        // get and set dimensions after packing
        atlas_packer.getDimensions(config.m_width, config.m_height);
        config.m_em_size = atlas_packer.getScale();
        config.m_pixel_range = atlas_packer.getPixelRange();

        KB_CORE_INFO("[font]: Glyph size: {} pixels / EM", config.m_em_size);
        KB_CORE_INFO("[font]: Atlas dimensions ({}, {})", config.m_width, config.m_height);
    }

    // edge coloring
    {
        if (config.m_image_type == msdf_atlas::ImageType::MSDF || config.m_image_type == msdf_atlas::ImageType::MTSDF)
        {
            if (config.m_expensive_coloring)
            {
                msdf_atlas::Workload(
    [&glyphs = m_msdf_metrics->m_glyphs, &config](i32 p_index, i32 p_thread_number) -> bool
                    {
                        const u64 glyph_seed = (k_lcg_multiplier * (config.m_coloring_seed ^ p_index) + k_lcg_increment) * !!config.m_coloring_seed;
                        glyphs[p_index].edgeColoring(config.m_edge_coloring_func, config.m_angle_threshold, glyph_seed);
                        return true;
                    },
                    static_cast<i32>(m_msdf_metrics->m_glyphs.size())
                ).finish(k_default_thread_count);
            }
            else
            {
                u64 glyph_seed = config.m_coloring_seed;
                for (auto& glyph : m_msdf_metrics->m_glyphs)
                {
                    glyph_seed *= k_lcg_multiplier;
                    glyph.edgeColoring(config.m_edge_coloring_func, config.m_angle_threshold, glyph_seed);
                }
            }
        }
    }

    // check for cached atlas
    details::atlas_header atlas_header{};
    owning_buffer storage_buffer{};
    // #NOTE: non owning pointer
    void* pixel_data = nullptr;
    if (details::try_read_cached_font_atlas(m_name, static_cast<f32>(config.m_em_size), atlas_header, pixel_data, storage_buffer))
    {
        m_texture_atlas = details::create_atlas(atlas_header, pixel_data);
        storage_buffer.Release();
    }
    else
    {
        constexpr bool use_floating_point = true;
        ref<Texture2D> texture_atlas{};
        switch (config.m_image_type)
        {
        case msdf_atlas::ImageType::MSDF:
            if (use_floating_point)
            {
                texture_atlas = details::create_and_cache_atlas<f32, f32, 3, msdf_atlas::msdfGenerator>(
                    m_name,
                    static_cast<f32>(config.m_em_size),
                    m_msdf_metrics->m_glyphs,
                    m_msdf_metrics->m_font_geometry,
                    config
                );
            }
            else
            {
                texture_atlas = details::create_and_cache_atlas<msdf_atlas::byte, f32, 3, msdf_atlas::msdfGenerator>(
                    m_name,
                    static_cast<f32>(config.m_em_size),
                    m_msdf_metrics->m_glyphs,
                    m_msdf_metrics->m_font_geometry,
                    config
                );
            }
            break;
        case msdf_atlas::ImageType::MTSDF:
            if (use_floating_point)
            {
                texture_atlas = details::create_and_cache_atlas<f32, f32, 4, msdf_atlas::mtsdfGenerator>(
                    m_name,
                    static_cast<f32>(config.m_em_size),
                    m_msdf_metrics->m_glyphs,
                    m_msdf_metrics->m_font_geometry,
                    config
                );
            }
            else
            {
                texture_atlas = details::create_and_cache_atlas<msdf_atlas::byte, f32, 4, msdf_atlas::mtsdfGenerator>(
                    m_name,
                    static_cast<f32>(config.m_em_size),
                    m_msdf_metrics->m_glyphs,
                    m_msdf_metrics->m_font_geometry,
                    config
                );
            }
            break;
        default:
            KB_CORE_ASSERT(false, "[font]: Failed to create texture atlas, unhandled ImageFormat={}", static_cast<u32>(config.m_image_format));
            break;
        }

        m_texture_atlas = texture_atlas;
    }
}

} // end namespace kb::render
