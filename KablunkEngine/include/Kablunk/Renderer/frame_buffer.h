#ifndef KABLUNK_RENDERER_FRAMEBUFFER_H
#define KABLUNK_RENDERER_FRAMEBUFFER_H

#include "Kablunk/Core/Core.h"
#include "Kablunk/Renderer/Image.h"
#include "Kablunk/Renderer/RendererTypes.h"
#include "Kablunk/Renderer/Texture.h"
#include <map>

namespace kb::render
{ // start namespace kb::render
class frame_buffer;

enum class frame_buffer_blend_mode_t
{
	none = 0,
	one_zero,
	src_alpha_one_minus_src_alpha,
	additive,
	zero_src_color
};

enum class attachment_load_op_t
{
    inherit = 0,
    clear,
    load
};

struct frame_buffer_texture_specification
{
	frame_buffer_texture_specification() = default;

	frame_buffer_texture_specification(ImageFormat f)
		: format{ f } { }

	ImageFormat format;
	bool blend = true;
	frame_buffer_blend_mode_t blend_mode = frame_buffer_blend_mode_t::src_alpha_one_minus_src_alpha;
    attachment_load_op_t m_load_op = attachment_load_op_t::inherit;
	// #TODO filtering and wrap
};

struct frame_buffer_attachment_specification
{
	frame_buffer_attachment_specification() = default;

	frame_buffer_attachment_specification(
        const std::initializer_list<frame_buffer_texture_specification>& attachments
    ) : Attachments{ attachments } {}

	std::vector<frame_buffer_texture_specification> Attachments;
};

// Properties struct
struct frame_buffer_specification
{
    // scale of the frame buffer
	float m_scale = 1.0f;
    // set width and heigth of the frame buffer
	uint32_t m_width = 0, m_height = 0;
    // frame buffer attachment specification (color, depth, etc.)
	frame_buffer_attachment_specification m_attachments;
    // ?
	uint32_t m_samples = 1;
    // clear color of the frame buffer
	glm::vec4 m_clear_color = { 0.0f, 0.0f, 0.0f, 1.0f };
    // flag to set the frame buffer to clear during the beginning of a render pass
	bool m_clear_color_on_load = true;
    // flag to set the frame buffer to clear depth during the beginning of a render apss
    bool m_clear_depth_on_load = true;
    // flag to allow frame buffer resizing
	bool m_no_resize = false;
    // flag to allow transparency blending should be enabled
	bool m_enable_blend = true;
    // transparency m_enable_blend mode
	frame_buffer_blend_mode_t m_blend_mode = frame_buffer_blend_mode_t::none;
    // flag to set frame buffer as a swap chain target
	bool m_swap_chain_target = false;
    // flag to set attachment images with transfer bits set
    bool m_transfer = false;
    // ?
	arc<Image2D> m_existing_image;
    // set existing image layers
	std::vector<uint32_t> m_existing_image_layers;
    // set existing attachment image(s)
	std::map<uint32_t, arc<Image2D>> m_existing_images;
    // target an existing frame buffer
	arc<frame_buffer> m_existing_frame_buffer;
    // debug name of the frame buffer
	std::string m_debug_name;
};

class frame_buffer : public RefCounted
{
public:
	~frame_buffer() override = default;

	virtual void resize(uint32_t width, uint32_t height, bool force_recreate = false) = 0;
	virtual void add_resize_callback(const std::function<void(arc<frame_buffer>)>& func) = 0;

	[[deprecated]] virtual void bind() const = 0;
    [[deprecated]] virtual void unbind() const = 0;

    [[deprecated]] virtual void bind_texture(uint32_t attachment_index = 0, uint32_t slot = 0) const = 0;

	virtual uint32_t get_width() const = 0;
	virtual uint32_t get_height() const = 0;
	virtual RendererID get_renderer_id() const = 0;

	virtual arc<Image2D> get_image(uint32_t attachment_index = 0) const = 0;
	virtual arc<Image2D> get_depth_image() const = 0;
	// #TODO clean up api because this is currently hard coded for reading an int from the buffer
    [[deprecated]] virtual int read_pixel(uint32_t attachment_index, int x, int y) = 0;

    [[deprecated]] virtual void clear_attachment(uint32_t attachment_index, int value) = 0;

	virtual const frame_buffer_specification& get_specification() const = 0;

    [[nodiscard]] virtual u32 get_color_attachment_count() const noexcept = 0;
    [[nodiscard]] virtual bool has_depth_attachment() const noexcept = 0;

	static arc<frame_buffer> create(const frame_buffer_specification& specs);
};

// why tf did I write this?
class frame_buffer_pool
{
public:
	frame_buffer_pool(uint32_t max_framebuffers = 32);
	~frame_buffer_pool();

	std::weak_ptr<frame_buffer> AllocateBuffer();
	void Add(const arc<frame_buffer>& framebuffer);

	std::vector<arc<frame_buffer>>& GetAll() { return m_pool; }
	const std::vector<arc<frame_buffer>> &GetAll() const { return m_pool; }

	inline static frame_buffer_pool* Get() { return s_instance; }
private:
	std::vector<arc<frame_buffer>> m_pool;

	static frame_buffer_pool* s_instance;
};
} // end namespace kb::render

#endif
