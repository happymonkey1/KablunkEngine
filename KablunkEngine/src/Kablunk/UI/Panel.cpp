#include "kablunkpch.h"
#include "Kablunk/UI/Panel.h"

#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Events/MouseEvent.h"

#include "Kablunk/Renderer/SceneRenderer.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace Kablunk::ui
{

	Panel::Panel(const glm::vec2& position, const glm::vec2& size, uuid::uuid64 id)
		: m_position{ position }, m_size{ size }, m_id{ id }
	{

	}

	Panel::~Panel()
	{
		destroy();
	}

	glm::vec2 Panel::get_position_relative() const
	{
		// #TODO include scale?
		glm::vec2 position = m_position;

		if (m_parent)
			position += m_parent->get_position_relative();

		return position;
	}

	const std::vector<IPanel*> Panel::get_children() const
	{
		return m_children;
	}

	void Panel::destroy()
	{
		for (IPanel* child : m_children)
		{
			child->destroy();
			delete child;
		}

		m_children.clear();
	}

	void Panel::on_event(Event& event)
	{
		for (IPanel* panel : m_children)
			if (event.GetStatus())
				panel->on_event(event);
		
	}

	void Panel::on_update(Timestep ts)
	{
		for (IPanel* panel : m_children)
			panel->on_update(ts);

		// mouse event and mouse inside viewport?
		bool mouse_down = input::is_mouse_button_pressed(Mouse::ButtonLeft) || input::is_mouse_button_pressed(Mouse::ButtonRight);
		if (mouse_down && input::is_mouse_in_viewport())
		{
			// get mouse pos relative to viewport
			auto [x, y] = input::get_mouse_position_relative_to_viewport();
			glm::vec2 mouse_pos = glm::vec2{ x, y };

			glm::vec2 pos = get_position_relative() + glm::vec2{ 0.5f } *render::get_viewport_size();
			// #TODO figure out how to convert kablunk units to pixel size
			glm::vec2 size = get_size();

			bool x_true = mouse_pos.x >= pos.x && mouse_pos.x <= pos.x + size.x;
			bool y_true = mouse_pos.y >= pos.y && mouse_pos.y <= pos.y + size.y;

			if (x_true && y_true)
			{
				if (input::is_mouse_button_pressed(Mouse::ButtonLeft))
					for (IPanel* panel : m_children)
						panel->on_left_mouse_down();

				if (input::is_mouse_button_pressed(Mouse::ButtonRight))
					for (IPanel* panel : m_children)
						panel->on_right_mouse_down();

				// #TODO figure out better way to send key events, instead of when mouse hovers the panel
				// #TODO key events
			}
		}
	}

	void Panel::on_render(const SceneRendererCamera& scene_camera)
	{
		// #TODO assert that this is called from within a render2d scene
		// KB_CORE_ASSERT(render2d::is_scene_rendering(), "did you forget to call render2d::begin_scene(...)?");

		if (!m_visible)
			return;

		glm::mat4 camera_proj = scene_camera.camera.GetProjection();
		glm::mat4 camera_transform = scene_camera.view_mat;

		ref<Texture2D> white_texture = render2d::get_white_texture();
		// #TODO camera_projection should probably not be here. if it is not, then the panel will not render in the editor
		glm::vec3 pos_relative_to_cam = glm::vec4{ get_position_relative().x, get_position_relative().y, 1.0f, 1.0f } * camera_proj;
		if (m_panel_style.render_background)
			render2d::draw_quad_ui(pos_relative_to_cam, m_size, white_texture, 1.0f, m_panel_style.background_color);

		if (m_panel_style.image)
			render2d::draw_quad_ui(pos_relative_to_cam, m_size, m_panel_style.image);

		for (IPanel* child : m_children)
			child->on_render(scene_camera);
	}

	void Panel::on_left_mouse_down()
	{

	}

	void Panel::on_right_mouse_down()
	{

	}

	void Panel::on_key_down(KeyCode key)
	{

	}

	glm::vec4 Panel::calculate_bounding_box() const
	{
		KB_CORE_ASSERT(false, "not implemented!");
		return glm::vec4{ 1.0f };
	}

	void Panel::add_child(IPanel* panel)
	{
		// #TODO check if we have already added this panel

		m_children.push_back(panel);
	}

	void Panel::remove_child(IPanel* panel)
	{
		if (!panel)
		{
			KB_CORE_ERROR("[Panel]: Trying to remove nullptr?");
			return;
		}

		// should this recursively check?
		auto it = std::find(m_children.begin(), m_children.end(), panel);
		KB_CORE_ASSERT(it != m_children.end(), "[Panel]: Tried removing panel '{}' but was not found in child list!", panel->get_panel_id());
		
		m_children.erase(it);
	}

	void Panel::set_parent(IPanel* parent)
	{
		if (!parent)
			KB_CORE_WARN("[Panel]: setting panel '{}' parent to null!");

		m_parent = parent;
	}

}
