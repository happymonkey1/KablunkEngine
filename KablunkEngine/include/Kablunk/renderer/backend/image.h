#pragma once
#ifndef KABLUNK_RENDERER_IMAGE_H
#define KABLUNK_RENDERER_IMAGE_H


#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/owning_buffer.h"
#include <glm/glm.hpp>

#include "Kablunk/renderer/backend/render_resource.h"

namespace kb::render::backend
{ // start namespace kb::render::backend

enum class image_format_t
{
	None = 0,
	RED32I = 1,
	RED32F = 2,
	RGB = 3,
	RGBA = 4,
	RGBA16F = 5,
	RGBA32F = 6,
	RG16F = 7,
	RG32F = 8,

	SRGB = 9,

	DEPTH32F = 10,
	DEPTH24STENCIL8 = 11,
    DEPTH32FSTENCIL8UINT = 12,

    // internal use only
    END = 13,

	// Defaults
	Depth = DEPTH24STENCIL8,
};

enum class image_usage_t
{
	None = 0,
	Texture,
	Attachment,
	Storage
};

enum class texture_wrap_t
{
	None = 0,
	Clamp,
	Repeat
};

enum class texture_filter_t
{
	None = 0,
	Linear,
	Nearest
};

enum class texture_type_t
{
	None = 0,
	Texture2D,
	TextureCube
};

struct texture_properties_t
{
	texture_wrap_t sampler_wrap = texture_wrap_t::Repeat;
	texture_filter_t sampler_filter = texture_filter_t::Linear;
	bool generate_mips = true;
	bool SRGB = false;
	bool Storage = false;

	std::string debug_name;
};

struct image_specification_t
{
	image_format_t format = image_format_t::RGBA;
	image_usage_t usage = image_usage_t::Texture;
    u32 width = 1;
    u32 height = 1;
    u32 mips = 1;
    u32 layers = 1;
	bool deinterleaved = false;
    bool m_transfer = false;

	std::string debug_name;
};

class image : public render_resource
{
public:
    ~image() override = default;

	virtual void invalidate() = 0;
	virtual void release() = 0;

	virtual u32 get_width() const = 0;
	virtual u32 get_height() const = 0;
    virtual glm::uvec2 get_size() const = 0;
	virtual float get_aspect_ratio() const = 0;

	virtual image_specification_t& get_specification() = 0;
	virtual const image_specification_t& get_specification() const = 0;

	virtual const owning_buffer& get_buffer() const = 0;
	virtual owning_buffer& get_buffer() = 0;

	virtual void create_per_layer_image_views() = 0;

	virtual u64 get_hash() const = 0;

	// TODO: usage (eg. shader read)
};

class image_2d : public image
{
public:
	static arc<image_2d> create(const image_specification_t& specification, owning_buffer buffer);
	static arc<image_2d> create(const image_specification_t& specification, const void* data = nullptr);
};

struct image_view_specification
{
    arc<image_2d> m_image{};
    u32 m_mip = 0;
    std::string m_debug_name{};
};

class image_view : public render::render_resource
{
public:
    ~image_view() override = default;

    static auto create(const image_view_specification& p_specification) noexcept -> arc<image_view>;
};

namespace util
{ // namespace ::util

inline uint32_t GetImageFormatBPP(image_format_t format)
{
    switch (format)
    {
    case image_format_t::RED32I:   return 4;
    case image_format_t::RED32F:   return 4;
    case image_format_t::RGB:
    case image_format_t::SRGB:     return 3;
    case image_format_t::RGBA:     return 4;
    case image_format_t::RGBA16F:  return 2 * 4;
    case image_format_t::RGBA32F:  return 4 * 4;
    }
    KB_CORE_ASSERT(false, "Unknown ImageFormat!");
    return 0;
}

inline u32 CalculateMipCount(u32 width, u32 height)
{
    return static_cast<u32>(std::floor(std::log2(glm::min(width, height)))) + 1;
}

inline u32 GetImageMemorySize(image_format_t format, u32 width, u32 height)
{
    return width * height * GetImageFormatBPP(format);
}

inline bool IsDepthFormat(image_format_t format)
{
    if (format == image_format_t::DEPTH24STENCIL8 || format == image_format_t::DEPTH32F || format == image_format_t::DEPTH32FSTENCIL8UINT)
        return true;

    return false;
}

} // end namespace ::util

} // end namespace kb::render::backend

#endif
