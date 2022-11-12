#pragma once
#ifndef KABLUNK_UI_IPANEL_H
#define KABLUNK_UI_IPANEL_H

#include "Kablunk/Events/Event.h"
#include "Kablunk/Core/Timestep.h"
#include "Kablunk/Core/KeyCodes.h"
#include "Kablunk/Core/Uuid64.h"

#include "Kablunk/Utilities/EnumIterator.hpp"

#include <glm/glm.hpp>

#include <vector>

// forward declaration
namespace Kablunk
{
	struct SceneRendererCamera;
	class Texture2D;
}

namespace Kablunk::ui
{

	enum class panel_type_t : u32
	{
		Blank,
		ImageButton,
		NONE
	};

	using panel_type_iterator = util::enum_iterator<panel_type_t, panel_type_t::Blank, panel_type_t::NONE>;

	// convert panel type to human readable cstr
	const char* panel_type_to_c_str(panel_type_t panel_type);

	struct panel_style_t
	{
		glm::vec4 background_color = glm::vec4{ 1.0f };
		bool render_background = true;
		glm::vec4 highlight_color = glm::vec4{ 0.9f };
		ref<Texture2D> image = nullptr;
	};

	// abstract base class for a ui panel
	class IPanel
	{
	public:
		virtual ~IPanel() = default;
		// get the size of this panel
		virtual const glm::vec2& get_size() const = 0;
		// get the size of this panel
		virtual glm::vec2& get_size() = 0;
		// get an absolute position for this panel
		virtual const glm::vec2& get_position() const = 0;
		// get the absolute position for this panel
		virtual glm::vec2& get_position() = 0;
		// get the unique identifier for this panel
		virtual uuid::uuid64 get_panel_id() const = 0;
		// get a relative position for this panel, relative to its parents
		virtual glm::vec2 get_position_relative() const = 0;
		// get the underlying panel type
		virtual panel_type_t get_panel_type() const = 0;

		// ========
		// children
		// ========

		// get a list of panels attached to this one
		virtual const std::vector<IPanel*> get_children() const = 0;
		// add a child panel to this one
		virtual void add_child(IPanel* panel) = 0;
		// remove a child panel from this one
		virtual void remove_child(IPanel* panel) = 0;
		// get the parent for this panel
		virtual IPanel* get_parent() const = 0;
		// set the parent for this panel
		virtual void set_parent(IPanel* parent) = 0;

		// =========
		// rendering
		// =========

		// check whether this panel is marked as visible
		virtual bool is_visible() const = 0;
		// set whether this panel is visible
		virtual void set_visible(bool visibility) = 0;
		// set the color of the background for this panel
		virtual void set_bg_color(const glm::vec4& color) = 0;
		// get the color of the background for this panel
		virtual const glm::vec4& get_bg_color() = 0;
		// get the panel style struct
		virtual const panel_style_t& get_panel_style() const = 0;
		// get the panel style struct
		virtual panel_style_t& get_panel_style() = 0;


		// =========

		virtual void on_event(Event& event) = 0;

		virtual void on_update(Timestep ts) = 0;

		// rendering code for the panel
		// must be called inside of a render2d scene
		virtual void on_render(const SceneRendererCamera& scene_camera) = 0;

		// destroy this panel and all its children
		virtual void destroy() = 0;

		// ======
		// events
		// ======

		virtual void on_left_mouse_down() = 0;
		virtual void on_right_mouse_down() = 0;
		virtual void on_key_down(KeyCode key) = 0;
	protected:
		// set the panel type
		virtual void set_panel_type(panel_type_t panel_type) = 0;
		// set panel position
		virtual void set_position(const glm::vec2& position) = 0;
		// set panel size
		virtual void set_size(const glm::vec2& size) = 0;
		// set panel style
		virtual void set_style(const panel_style_t& panel_style) = 0;
	private:
		friend class PanelFactory;
	};

}

#endif
