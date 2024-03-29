#pragma once
#ifndef KABLUNK_RENDERER_RENDER_COMMAND_2D_H
#define KABLUNK_RENDERER_RENDER_COMMAND_2D_H


// singleton renderer2d "render commands" deprecated, and moved to a dependency injection model
#if 0

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Scene/Entity.h"

namespace kb::render2d
{
	// #TODO refactor public api
	
	// initialize renderer 2D, with optional specification
	static inline void init(renderer_2d_specification_t spec = {}) { Singleton<Renderer2D>::get().init(spec); }

	// shutdown renderer 2d and free resources
	static inline void shutdown() { Singleton<Renderer2D>::get().shutdown(); }

	// get 1x1 white texture
	static inline IntrusiveRef<Texture2D> get_white_texture() { return Singleton<Renderer2D>::get().get_white_texture(); }

	// begin renderer2d scene with a render camera and transform
	static inline void begin_scene(const Camera& camera, const glm::mat4& transform) { Singleton<Renderer2D>::get().begin_scene(camera, transform); }

	// end renderer2d scene
	static inline void end_scene() { Singleton<Renderer2D>::get().end_scene(); }

	// get the target render pass
	static inline IntrusiveRef<RenderPass> get_target_render_pass() { Singleton<Renderer2D>::get().get_target_render_pass(); }

	// set the target render pass
	static inline void set_target_render_pass(IntrusiveRef<RenderPass> render_pass) { Singleton<Renderer2D>::get().set_target_render_pass(render_pass); }

	// draw a sprite
	static inline void draw_sprite(kb::Entity entity) { Singleton<Renderer2D>::get().draw_sprite(entity); }

	// draw a quad with vec2 position
	static inline void draw_quad(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_quad(position, size, texture, tiling_factor, tint_color);
	}

	// draw a quad with vec3 position
	static inline void draw_quad(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_quad(position, size, texture, tiling_factor, tint_color);
	}

	// draw a ui quad (in screen space) with vec2 position
	static inline void draw_quad_ui(const glm::vec2& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_quad_ui(position, size, texture, tiling_factor, tint_color);
	}

	// draw a ui quad (in screen space) with vec2 position
	static inline void draw_quad_ui(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture, float tiling_factor = 1.0f, const glm::vec4& tint_color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_quad_ui(position, size, texture, tiling_factor, tint_color);
	}

	// draw a rect with a 2d position
	static inline void draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_rect(position, size, rotation, color);
	}

	// draw a rect with a 3d position
	static inline void draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_rect(position, size, rotation, color);
	}

	// draw a circle
	static inline void draw_circle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f,
							float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1) 
	{ 
		Singleton<Renderer2D>::get().draw_circle(transform, color, radius, thickness, fade, entity_id); 
	}

	// draw a texture from a texture atlas (single, combined texture)
	static inline void draw_quad_from_texture_atlas(const glm::vec3& position, const glm::vec2& size, const IntrusiveRef<Texture2D>& texture,
											 const glm::vec2* texture_atlas_offsets, float tiling_factor = 1.0f, 
											 const glm::vec4& tint_color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_quad_from_texture_atlas(position, size, texture, texture_atlas_offsets, tiling_factor, tint_color);
	}

    // draw 2d text string to screen
    static inline void draw_text_string(
        const std::string& text,
        const glm::vec2& position,
        const glm::vec2& size,
        const ref<render::font_asset_t>& font_asset,
        const glm::vec4& tint_color = glm::vec4{ 1.0f }
    )
    {
        Singleton<Renderer2D>::get().draw_text_string(text, position, size, font_asset, tint_color);
    }

    // draw 2d text string to screen
    static inline void draw_text_string(
        const std::string& text,
        const glm::vec3& position,
        const glm::vec2& size,
        const ref<render::font_asset_t>& font_asset,
        const glm::vec4& tint_color = glm::vec4{ 1.0f }
    )
    {
        Singleton<Renderer2D>::get().draw_text_string(text, position, size, font_asset, tint_color);
    }

	// get renderer 2d statistics
	static inline renderer_2d_stats_t get_stats() { return Singleton<Renderer2D>::get().get_stats(); }

	// reset renderer 2d statistics
	// #TODO client should not have to reset renderer stats
	static inline void reset_stats() { Singleton<Renderer2D>::get().reset_stats(); }

	// render imgui 
	static inline void on_imgui_render() { Singleton<Renderer2D>::get().on_imgui_render(); }

	// get the 2D font manager
	static inline render::font_manager& get_font_manager() { return Singleton<Renderer2D>::get().get_font_manager(); }
}

#endif

#endif
