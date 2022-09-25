#pragma once
#ifndef KABLUNK_RENDERER_RENDER_COMMAND_2D_H
#define KABLUNK_RENDERER_RENDER_COMMAND_2D_H

#include "Kablunk/Core/Singleton.h"
#include "Kablunk/Renderer/Renderer2D.h"
#include "Kablunk/Scene/Entity.h"

namespace Kablunk::render2d
{
	// #TODO refactor public api
	
	// initialize renderer 2D, with optional specification
	static void init(renderer_2d_specification_t spec = {}) { Singleton<Renderer2D>::get().init(spec); }

	// shutdown renderer 2d and free resources
	static void shutdown() { Singleton<Renderer2D>::get().shutdown(); }

	// get 1x1 white texture
	static IntrusiveRef<Texture2D> get_white_texture() { return Singleton<Renderer2D>::get().get_white_texture(); }

	// begin renderer2d scene with a view projection
	static void begin_scene(const glm::mat4& view_proj) { Singleton<Renderer2D>::get().begin_scene(view_proj); }

	// begin renderer2d scene with a render camera and transform
	static void begin_scene(const Camera& camera, const glm::mat4& transform) { Singleton<Renderer2D>::get().begin_scene(camera, transform); }

	// end renderer2d scene
	static void end_scene() { Singleton<Renderer2D>::get().end_scene(); }

	// get the target render pass
	static IntrusiveRef<RenderPass> get_target_render_pass() { Singleton<Renderer2D>::get().get_target_render_pass(); }

	// set the target render pass
	static void set_target_render_pass(IntrusiveRef<RenderPass> render_pass) { Singleton<Renderer2D>::get().set_target_render_pass(render_pass); }

	// draw a sprite
	static void draw_sprite(Kablunk::Entity entity) { Singleton<Renderer2D>::get().draw_sprite(entity); }

	// draw a rect with a 2d position
	static void draw_rect(const glm::vec2& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_rect(position, size, rotation, color);
	}

	// draw a rect with a 3d position
	static void draw_rect(const glm::vec3& position, const glm::vec2& size, float rotation = 0, const glm::vec4& color = glm::vec4{ 1.0f })
	{
		Singleton<Renderer2D>::get().draw_rect(position, size, rotation, color);
	}

	// draw a circle
	static void draw_circle(const glm::mat4& transform, const glm::vec4& color, float radius = 0.5f, 
							float thickness = 1.0f, float fade = 0.005f, int32_t entity_id = -1) 
	{ 
		Singleton<Renderer2D>::get().draw_circle(transform, color, radius, thickness, fade, entity_id); 
	}

	// get renderer 2d statistics
	static renderer_2d_stats_t get_stats() { return Singleton<Renderer2D>::get().get_stats(); }

	// reset renderer 2d statistics
	// #TODO client should not have to reset renderer stats
	static void reset_stats() { Singleton<Renderer2D>::get().reset_stats(); }

	// render imgui 
	static void on_imgui_render() { Singleton<Renderer2D>::get().on_imgui_render(); }
}

#endif
