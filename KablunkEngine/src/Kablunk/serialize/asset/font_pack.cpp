#include "kablunkpch.h"
#include "Kablunk/serialize/asset/font_pack.h"
#include "Kablunk/Core/md5_hash.h"

namespace kb::asset
{ // start namespace kb::asset

font_pack::font_pack(std::filesystem::path p_filepath)
    : m_path{ std::move(p_filepath) }, m_font_pack{}, m_font_ids{}
{
    init_reserve_space();
}

auto font_pack::add_font(const ref<render::font>& p_font_asset) noexcept -> void
{
    const auto asset_id = p_font_asset->get_id();

    if (m_font_ids.contains(asset_id))
    {
        KB_CORE_WARN("[font_pack]: Tried adding font asset '{}' to font pack, but it is already included.", asset_id);
        return;
    }

    m_font_ids.insert(asset_id);

    const auto& font_atlas = p_font_asset->get_font_atlas();
    // add header
    m_font_pack.m_font_atlas_headers.emplace_back(
        font_atlas_header{
            .m_image_type = 0,
            .m_width = font_atlas->GetWidth(),
            .m_height = font_atlas->GetHeight(),
            .m_checksum = 0,
        }
    );
    // add font asset
    m_font_pack.m_fonts.emplace_back(p_font_asset);
    // increment count
    m_font_pack.m_header.m_atlas_count++;
}


auto font_pack::load(const std::filesystem::path& p_filepath) noexcept -> std::unique_ptr<font_pack>
{
    std::ifstream font_pack_file{ p_filepath.string().c_str(), std::ios::binary | std::ios::in };

    KB_CORE_ASSERT(font_pack_file.is_open(), "[font_pack]: Failed to open '{}'", p_filepath.string().c_str());

    // read file into local buffer
    const auto buf_size = static_cast<size_t>(font_pack_file.tellg());
    font_pack_file.seekg(0);
    owning_buffer font_pack_buf{ buf_size };
    font_pack_file.read(static_cast<char*>(font_pack_buf.data()), static_cast<i64>(buf_size));
    KB_CORE_INFO("[font_pack]: Read {} bytes from '{}'", buf_size, p_filepath.string().c_str());

    u8* buf_cursor = static_cast<u8*>(font_pack_buf.get());

    auto font_pack = std::make_unique<asset::font_pack>(p_filepath);
    const font_pack_header* header = reinterpret_cast<asset::font_pack_header*>(buf_cursor);
    // copy header
    font_pack->m_font_pack.m_header = *header;
    KB_CORE_ASSERT(
        font_pack->m_font_pack.m_header.is_valid(),
        "[font_pack]: Failed to deserialize font asset pack header!"
    );
    buf_cursor += sizeof(font_pack_header);

    const auto atlas_count = font_pack->m_font_pack.m_header.m_atlas_count;
    KB_CORE_INFO("[font_pack]: Font asset pack header says there are {} atlases", atlas_count);

    for (size_t i = 0; i < atlas_count; ++i)
    {
        KB_CORE_INFO("[font_pack]: Starting to deserialize font atlas {}", i);
        font_pack->deserialize_font(buf_cursor, i);
    }

    return font_pack;
}

auto font_pack::save() const noexcept -> void
{
    const size_t asset_pack_size = sizeof(font_pack_header) +
        (sizeof(font_atlas_header) * m_font_pack.m_fonts.size()) +
        std::accumulate(
            m_font_pack.m_fonts.begin(),
            m_font_pack.m_fonts.end(),
            0ull,
            [](size_t p_val, const ref<render::font>& p_font_asset) -> size_t
            {
                const auto& font_atlas = p_font_asset->get_font_atlas();
                return p_val + Utils::GetImageMemorySize(font_atlas->GetFormat(), font_atlas->GetWidth(), font_atlas->GetHeight());
            }
        );

    const auto font_pack_buf_head = new u8[asset_pack_size];
    auto buf_cursor = font_pack_buf_head;

    const auto header_ptr = reinterpret_cast<font_pack_header*>(buf_cursor);
    *header_ptr = font_pack_header{
        .m_magic_header = { font_pack_header::k_magic_header[0], font_pack_header::k_magic_header[1], font_pack_header::k_magic_header[2], font_pack_header::k_magic_header[3], },
        .m_version = font_pack_header::k_version,
        .m_atlas_count = m_font_pack.m_fonts.size(),
    };
    buf_cursor += sizeof(font_pack_header);

    for (const auto& font_asset : m_font_pack.m_fonts)
        serialize_font(buf_cursor, font_asset);

    std::ofstream file{ m_path, std::ios::binary | std::ios::trunc };
    file.write(reinterpret_cast<char*>(font_pack_buf_head), static_cast<i64>(asset_pack_size));

    KB_CORE_INFO("[font_pack]: Wrote {} bytes to '{}'", asset_pack_size, m_path.string().c_str());

    delete[] font_pack_buf_head;
}

auto font_pack::serialize_font(
    u8*& p_cursor,
    const ref<render::font>& p_font_asset
) noexcept -> void
{
    const auto& font_atlas = p_font_asset->get_font_atlas();
    const auto header_ptr = reinterpret_cast<font_atlas_header*>(p_cursor);

    const auto& atlas_data_buffer = font_atlas->get_buffer();

    *header_ptr = font_atlas_header{
        .m_image_type = static_cast<u32>(font_atlas->GetFormat()),
        .m_width = font_atlas->GetWidth(),
        .m_height = font_atlas->GetHeight(),
        .m_font_name = { 0 },
        .m_checksum = compute_md5_hash(atlas_data_buffer.get(), atlas_data_buffer.size())
    };

    // set name outside of constructor because of const char[] and char* shenanigans
    const auto font_name_substr = p_font_asset->get_name().substr(0, font_atlas_header::k_name_size - 1).data();
    for (size_t i = 0; i < font_atlas_header::k_name_size; ++i)
        header_ptr->m_font_name[i] = font_name_substr[i];

    p_cursor += sizeof(font_atlas_header);


    memcpy(p_cursor, atlas_data_buffer.get(), atlas_data_buffer.size());

    p_cursor += atlas_data_buffer.size();
}

auto font_pack::deserialize_font(
    u8*& p_cursor,
    const size_t p_atlas_index
) noexcept -> void
{
    const auto atlas_header_ptr = reinterpret_cast<font_atlas_header*>(p_cursor);
    // copy atlas header to font pack
    m_font_pack.m_font_atlas_headers.emplace_back(*atlas_header_ptr);
    KB_CORE_ASSERT(m_font_pack.m_font_atlas_headers.size() == p_atlas_index + 1, "[font_pack]: Index is not aligned with header array size?");

    const auto& atlas_header = m_font_pack.m_font_atlas_headers.at(p_atlas_index);
    p_cursor += sizeof(font_atlas_header);

    KB_CORE_ASSERT(atlas_header_ptr->m_image_type < static_cast<u32>(ImageFormat::END), "[font_pack]: Invalid font atlas image type {}", atlas_header_ptr->m_image_type);
    const auto image_format = static_cast<ImageFormat>(atlas_header.m_image_type);
    const auto atlas_size = Utils::GetImageMemorySize(image_format, atlas_header.m_width, atlas_header.m_height);

    // check md5 hashes match
    const auto& header_md5 = atlas_header.m_checksum;
    const auto& buffer_md5 = compute_md5_hash(p_cursor, atlas_size);
    KB_CORE_ASSERT(
        header_md5 == buffer_md5,
        "[font_pack]: md5 from header does not match from buffer"
    );

    // #TODO do we really need a copy here...?
    owning_buffer atlas_buffer{
        p_cursor,
        atlas_size
    };

    auto font = ref<render::font>::Create(atlas_header.m_font_name, std::move(atlas_buffer));
    m_font_pack.m_fonts.emplace_back(font);
    KB_CORE_ASSERT(m_font_pack.m_fonts.size() == p_atlas_index + 1, "[font_pack]: Index is not aligned with font array size?");

    p_cursor += atlas_size;
}

auto font_pack::init_reserve_space() noexcept -> void
{
    constexpr size_t k_expected_fonts = 16;
    m_font_pack.m_font_atlas_headers.reserve(k_expected_fonts);
    m_font_pack.m_fonts.reserve(k_expected_fonts);
    m_font_ids.reserve(k_expected_fonts);
}
} // end namespace kb::asset
