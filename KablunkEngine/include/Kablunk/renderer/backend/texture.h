#ifndef KABLUNK_RENDERER_TEXTURE_H
#define KABLUNK_RENDERER_TEXTURE_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/backend/image.h"

#include "Kablunk/Asset/Asset.h"

#include <string>

namespace kb::render::backend
{ // start namespace kb::render::backend

class texture : public render_resource
{
public:
	~texture() override = default;
	virtual u32 get_width() const = 0;
	virtual u32 get_height() const = 0;
	virtual void set_data(void* data, u32 size) = 0;

	virtual uint64_t get_hash() const = 0;

	virtual void bind(u32 slot = 0) const = 0;
};

class texture_2d : public texture
{
public:
    ~texture_2d() override = default;

	virtual void resize(u32 width, u32 height) = 0;
	virtual arc<image_2d> get_image() const = 0;

	virtual image_format_t get_format() const = 0;

	virtual owning_buffer& get_writeable_buffer() = 0;
    virtual const owning_buffer& get_buffer() const = 0;
	virtual bool operator==(const texture_2d& other) const = 0;

	virtual bool loaded() const = 0;

	// static method to get the asset type of the class
	static asset::AssetType get_static_type() { return asset::AssetType::Texture; }

	static arc<texture_2d> create(image_format_t format, u32 width, u32 height, const void* data = nullptr);
    static arc<texture_2d> create(const std::string& path);
private:
	virtual void invalidate() = 0;
};

} // end namespace kb::render::backend

#endif
