#pragma once
#ifndef KABLUNK_UI_PANEL_H
#define KABLUNK_UI_PANEL_H

#include "Kablunk/UI/IPanel.h"

namespace Kablunk::ui
{

	class Panel : public IPanel
	{
	public:
		Panel() = default;
		Panel(const glm::vec2& position, const glm::vec2& size, uuid::uuid64 id = uuid::generate());

		virtual ~Panel();

		// get the size of this panel
		virtual const glm::vec2& get_size() const override { return m_size; };

		// get the size of this panel
		virtual glm::vec2& get_size() override { return m_size; };

		// get an absolute position for this panel
		virtual const glm::vec2& get_position() const override { return m_position; };

		// get the absolute position for this panel
		virtual glm::vec2& get_position() override { return m_position; };

		// get the unique identifier for this panel
		virtual uuid::uuid64 get_panel_id() const override { return m_id; }

		// get a relative position for this panel, relative to its parents
		virtual glm::vec2 get_position_relative() const override;

		// get the underlying panel type
		virtual panel_type_t get_panel_type() const override { return m_panel_type; };

		// ========
		// children
		// ========

		// get a list of panels attached to this one
		virtual const std::vector<IPanel*> get_children() const override;

		// add a child panel to this one
		virtual void add_child(IPanel* panel) override;

		// remove a child panel from this one
		virtual void remove_child(IPanel* panel) override;

		// get the parent for this panel
		virtual IPanel* get_parent() const override { return m_parent; }
		// set the parent for this panel
		virtual void set_parent(IPanel* parent);

		// =========
		// rendering
		// =========

		// check whether this panel is marked as visible
		virtual bool is_visible() const override { return m_visible; };

		// set whether this panel is visible
		virtual void set_visible(bool visibility) override { m_visible = visibility; };

		// set the color of the background for this panel
		virtual void set_bg_color(const glm::vec4& color) override { m_panel_style.background_color = color; };

		// get the color of the background for this panel
		virtual const glm::vec4& get_bg_color() override { return m_panel_style.background_color; };

		// get the panel style struct
		virtual const panel_style_t& get_panel_style() const { return m_panel_style; };

		// get the panel style struct
		virtual panel_style_t& get_panel_style() { return m_panel_style; };

		// =========

		// destroy this panel and all of its children
		virtual void destroy() override;

		virtual void on_event(Event& event) override;

		virtual void on_update(Timestep ts) override;

		virtual void on_render(const SceneRendererCamera& scene_camera) override;

		// ======
		// events
		// ======

		virtual void on_left_mouse_down() override;
		virtual void on_right_mouse_down() override;
		virtual void on_key_down(KeyCode key) override;
	protected:
		// set the panel type
		virtual void set_panel_type(panel_type_t panel_type) override { m_panel_type = panel_type; };

		// set panel position
		virtual void set_position(const glm::vec2& position) override { m_position = position; };

		// set panel size
		virtual void set_size(const glm::vec2& size) override { m_size = size; };

		// set panel style
		virtual void set_style(const panel_style_t& panel_style) { m_panel_style = panel_style; };
	private:
		// return a vec4 containing the points for the bounding box of this panel
		glm::vec4 calculate_bounding_box() const;
	protected:
		// position of the panel
		glm::vec2 m_position;
		// size of the panel
		glm::vec2 m_size;
		// uuid for the panel
		uuid::uuid64 m_id;
		// list of all child elements for this panel
		std::vector<IPanel*> m_children;
		// pointer to the parent panel
		IPanel* m_parent = nullptr;
		// flag for whether this panel is visible
		bool m_visible = true;
		// struct that holds panel style
		panel_style_t m_panel_style;
		// underlying panel type
		panel_type_t m_panel_type;
	};

}

#endif
