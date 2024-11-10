#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_TEXTURE_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_TEXTURE_H

#include "Kablunk/Core/owning_buffer.h"

#include "Kablunk/renderer/backend/texture.h"
#include "Kablunk/renderer/backend/image.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_image.h"

#include <vulkan/vulkan.h>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

class vulkan_texture_2d final : public texture_2d
{
public:
	vulkan_texture_2d(
        weak_ptr<vulkan_logical_device> p_device,
        image_format_t format,
        u32 width,
        u32 height,
        const void* data
    );
	vulkan_texture_2d(weak_ptr<vulkan_logical_device> p_device, std::string path);
	~vulkan_texture_2d() override;

	void resize(u32 width, u32 height) override;
	arc<image_2d> get_image() const override { return m_image; }

	image_format_t get_format() const override { return m_format; }

    u32 get_width() const override { return m_width; }
    u32 get_height() const override { return m_height; }
	RendererID get_renderer_id() const override { return 0; }
	uint64_t get_hash() const override { return m_hash; }

    resource_descriptor_info_t get_descriptor_info() noexcept override
	{
        return m_image.As<vulkan_image_2d>()->get_descriptor_info();
	}

	const VkDescriptorImageInfo& GetVulkanDescriptorInfo() const
	{
	    return m_image.As<vulkan_image_2d>()->get_vk_image_info_descriptor();
	}

	owning_buffer& get_writeable_buffer() override;
    const owning_buffer& get_buffer() const override { return m_image_data; }

	bool loaded() const override { return m_loaded; }

	void set_data(void* data, u32 size) override;

	void bind(u32 slot) const override;
	bool operator==(const texture_2d& other) const override;
private:
	virtual void invalidate() override;
	bool load_image(const std::string& filepath);
private:
	std::string m_filepath;
    u64 m_hash = 0ull;
	u32 m_width;
    u32 m_height;

    weak_ptr<vulkan_logical_device> m_device = nullptr;
	arc<image_2d> m_image{};

	image_format_t m_format;

    // buffer of image data stored on cpu
	owning_buffer m_image_data;

	bool m_loaded = false;

    friend class vulkan_material;
};

} // end namespace kb::render::backend::vk

#endif
