#include "kablunkpch.h"
#include "Kablunk/Renderer/texture_atlas.h"

#include <stb_image.h>
#include <stb_image_write.h>

namespace kb::render
{ // start namespace kb::render

texture_atlas::texture_atlas(const texture_atlas_create_props& p_props) noexcept
    : m_filepath{ p_props.m_path }
{
    KB_ASSERT(!p_props.m_path.empty(), "[render::texture_atlas]: path is empty!");
    KB_ASSERT(!p_props.m_root_directory.empty(), "[render::texture_atlas]: root directory is empty!");

    const bool should_create = !std::filesystem::exists(p_props.m_path) || p_props.m_force_create;
    if (should_create)
        create_texture_atlas(p_props);
    else
    {
        KB_CLIENT_INFO(
            "[render::texture_atlas] Found cached texture atlas '{}'!",
            p_props.m_path.string().c_str()
        );
    }

}

// lightmap texture packing algorithm: https://blackpawn.com/texts/lightmaps/
// greedy bottom left packing
auto texture_atlas::node_t::insert(const image_data_t& p_image_data) noexcept -> node_t*
{
    if (!is_leaf())
    {
        if (node_t* new_node = m_left ? m_left->insert(p_image_data) : nullptr; new_node != nullptr)
            return new_node;

        return m_right->insert(p_image_data);
    }

    if (m_image_hash != 0)
        return nullptr;

    const bool can_fit = p_image_data.m_width <= static_cast<u32>(m_rect.get_width()) &&
        p_image_data.m_height <= static_cast<u32>(m_rect.get_height());
    if (!can_fit)
        return nullptr;

    const bool perfect_fit = p_image_data.m_width == static_cast<u32>(m_rect.get_width()) &&
        p_image_data.m_height == static_cast<u32>(m_rect.get_height());
    if (perfect_fit)
    {
        m_image_hash = static_cast<u64>(p_image_data.m_id);
        return this;
    }

    m_left = new node_t{};
    m_right = new node_t{};

    const auto dw = m_rect.get_width() - p_image_data.m_width;
    const auto dh = m_rect.get_height() - p_image_data.m_height;

    if (dw > dh)
    {
        m_left->m_rect = rect_i32{
            m_rect.m_left,
            m_rect.m_top,
            m_rect.m_left + static_cast<i32>(p_image_data.m_width),
            m_rect.m_bottom
        };

        m_right->m_rect = rect_i32{
            m_rect.m_left + static_cast<i32>(p_image_data.m_width),
            m_rect.m_top,
            m_rect.m_right,
            m_rect.m_bottom
        };
    }
    else
    {
        m_left->m_rect = rect_i32{
            m_rect.m_left,
            m_rect.m_top,
            m_rect.m_right,
            m_rect.m_top + static_cast<i32>(p_image_data.m_height)
        };

        m_right->m_rect = rect_i32{
            m_rect.m_left,
            m_rect.m_top + static_cast<i32>(p_image_data.m_height),
            m_rect.m_right,
            m_rect.m_bottom
        };
    }

    return m_left->insert(p_image_data);
}

auto texture_atlas::delete_tree(node_t* p_root) noexcept -> void
{
    if (!p_root)
        return;

    delete_tree(p_root->m_left);
    delete_tree(p_root->m_right);

    delete p_root;
}

auto texture_atlas::create_texture_atlas(
    const texture_atlas_create_props& p_props
) noexcept -> void
{
    // by default, search through asset directory
    // #TODO should probably be able to search multiple directories for modding support...
    const auto& root_directory = p_props.m_root_directory;

    std::vector<std::filesystem::path> directories_to_search{};
    directories_to_search.reserve(64ull);
    directories_to_search.emplace_back(root_directory);

    constexpr u32 atlas_width = k_default_atlas_size;
    m_atlas_dimension = atlas_width;

    // #TODO support hdr textures

    constexpr size_t buffer_size = static_cast<std::size_t>(atlas_width) *
        static_cast<std::size_t>(atlas_width) * image_data_t::k_bit_depth_data_size;
    owning_buffer atlas_image_buf{ buffer_size };
    atlas_image_buf.zero();

    m_uv_map.reserve(256ull);

    const auto root_insertion_node = new node_t{
        nullptr,
        nullptr,
        rect_i32{ 0l, 0l, k_default_atlas_size, k_default_atlas_size },
        0l,
    };

    // #TODO this is not very efficient, will scan all files multiple times if we need to create multiple atlases
    size_t dir_index = 0;
    while (dir_index < directories_to_search.size())
    {
        const auto& dir_path = directories_to_search.at(dir_index);
        // #TODO allocate large buffer once and re-use...
        for (const auto& entry : std::filesystem::directory_iterator(dir_path))
        {
            auto filename_str = entry.path().filename().string();
            // #TODO ignore pattern should probably be regex
            if (entry.is_directory() && filename_str[0] != '.')
            {
                directories_to_search.emplace_back(entry.path());
                continue;
            }

            // #TODO this should support more than just png
            if (!entry.is_regular_file() || entry.path().extension() != ".png")
                continue;

            auto image_data = load_image(entry.path());
            if (!image_data.is_valid())
                continue;

            const auto filename_as_str = entry.path().filename().string();
            KB_CLIENT_INFO("[render::texture_atlas]: adding '{}' to texture atlas", filename_as_str.c_str());

            const node_t* insert_node = root_insertion_node->insert(image_data);
            KB_ASSERT(insert_node, "[render::texture_atlas]: failed to insert into atlas!");

            add_image_to_atlas(insert_node, image_data, atlas_image_buf);
        }

        ++dir_index;
    }

    if (m_sprite_count == 0)
        KB_ASSERT(false, "[render::texture_atlas]: failed to find any sprites to add to atlas!");

    delete_tree(root_insertion_node);

    KB_CLIENT_INFO("[render::texture_atlas]: added {} textures to atlas", m_sprite_count);

    KB_CLIENT_TRACE("[render::texture_atlas]: saving tile atlas '{}'...", p_props.m_path.string().c_str());
    constexpr u32 k_channels = 4;


    const std::filesystem::path asset_path = std::filesystem::path{ "assets" } / m_filepath;
    stbi_write_png(
        asset_path.string().c_str(),
        atlas_width,
        atlas_width,
        k_channels,
        atlas_image_buf.get(),
        atlas_width * k_channels
    );
    KB_CLIENT_TRACE("[render::texture_atlas]: done saving");

    atlas_image_buf.release();
}

auto texture_atlas::add_image_to_atlas(
    const node_t* p_node,
    const image_data_t& p_image_data,
    owning_buffer& p_atlas_buffer
) noexcept -> void
{
    KB_ASSERT(p_node, "[render::texture_atlas]: atlas insertion node is null?");

    const u32 offset_y = p_node->m_rect.m_top, offset_x = p_node->m_rect.m_left;

    const size_t atlas_buf_offset_index = (offset_x)+(offset_y * m_atlas_dimension);

    void* atlas_image_buf_head = p_atlas_buffer.get();

    // copy image data to atlas buffer
    // iterates row by row and computes offset into atlas
    for (size_t sprite_buf_index = 0; sprite_buf_index < p_image_data.m_height; ++sprite_buf_index)
    {
        const u32 atlas_row_offset_index = (sprite_buf_index * m_atlas_dimension);
        const u32 image_row_offset_index = (sprite_buf_index * p_image_data.m_height);

        u32* dst = static_cast<u32*>(atlas_image_buf_head) + atlas_buf_offset_index + atlas_row_offset_index;
        const u32* src = static_cast<const u32*>(p_image_data.m_image_data.get()) + image_row_offset_index;

        KB_ASSERT(
            static_cast<const void*>(dst) < static_cast<void*>(static_cast<u8*>(atlas_image_buf_head) + p_atlas_buffer.size()),
            "[render::texture_atlas]: sprite image buffer overflow!"
        );

        KB_ASSERT(
            static_cast<const void*>(src) < static_cast<const void*>(static_cast<const u8*>(p_image_data.m_image_data.get()) + p_image_data.
                m_image_data.size()),
            "[render::texture_atlas]: sprite image buffer overflow!"
        );

        const std::size_t stride = p_image_data.m_height * image_data_t::k_bit_depth_data_size;
        // move?
        memcpy(dst, src, stride);
    }

    // calculate and store uvs
    calculate_uv_offsets(
        p_image_data.m_id,
        p_node->m_rect
    );

    ++m_sprite_count;
}

auto texture_atlas::load_image(const std::filesystem::path& p_path) noexcept -> image_data_t
{
    const std::string path_str = p_path.string();
    owning_buffer image_buffer{};
    i32 width, height, channels;
    void* data;
    if (stbi_is_hdr(path_str.c_str()))
    {
        KB_ASSERT(false, "[render::texture_atlas]: does not support hdr images!");
#if 0
        data = stbi_loadf(filepath.c_str(), &width, &height, &channels, 4);
        size_t size = static_cast<size_t>(width) * static_cast<size_t>(height) * 4ull * sizeof(float);
        image_buffer.Allocate(size);

        image_buffer.Write(data, size, 0);
        m_format = ImageFormat::RGBA32F;
#endif
        return {};
    }
    else
    {
        data = stbi_load(path_str.c_str(), &width, &height, &channels, 4);
        std::size_t size = static_cast<std::size_t>(width) * static_cast<std::size_t>(height) * 4ull;
        image_buffer.allocate(size);

        image_buffer.write(data, size, 0);
    }

    stbi_image_free(data);

    if (!image_buffer.get())
    {
        KB_ASSERT(false, "[render::texture_atlas]: Image loaded but data is null!");
        return {};
    }

    return {
        .m_image_data = image_buffer,
        .m_width = static_cast<u32>(width),
        .m_height = static_cast<u32>(height),
        .m_id = raw_texture_handle::into(std::string_view{ p_path.filename().string() }),
    };
}

auto texture_atlas::calculate_uv_offsets(raw_texture_handle p_id, const rect_i32& p_rect) noexcept -> void
{
    KB_ASSERT(
        !m_uv_map.contains(p_id),
        "[render::texture_atlas]: texture_id id '{}' is already in the uv map!",
        static_cast<u64>(p_id)
    );

    constexpr f32 border_uv_offset_x = 0.0f;
    constexpr f32 border_uv_offset_y = 0.0f;

    const f32 atlas_width = static_cast<f32>(m_atlas_dimension);
    const f32 atlas_height = static_cast<f32>(m_atlas_dimension);

    const f32 sprite_width = static_cast<f32>(p_rect.get_width());
    const f32 sprite_height = static_cast<f32>(p_rect.get_height());
    const f32 uv_width = sprite_width / atlas_width;
    const f32 uv_height = sprite_height / atlas_height;

    const std::array uvs{
        glm::vec2{
            static_cast<f32>(p_rect.m_left) / atlas_width,
            static_cast<f32>(p_rect.m_top) / atlas_height
        },
        glm::vec2{
            static_cast<f32>(p_rect.m_left) / atlas_width + uv_width,
            static_cast<f32>(p_rect.m_top) / atlas_height
        },
        glm::vec2{
            static_cast<f32>(p_rect.m_left) / atlas_width + uv_width,
            static_cast<f32>(p_rect.m_top) / atlas_height + uv_height
        },
        glm::vec2{
            static_cast<f32>(p_rect.m_left) / atlas_width,
            static_cast<f32>(p_rect.m_top) / atlas_height + uv_height
        }
    };

    m_uv_map.emplace(
        p_id,
        virtual_texture_data_t{
            uvs,
            glm::uvec2{ p_rect.get_width(), p_rect.get_height() }
        }
    );
}

} // end namespace kb::render
