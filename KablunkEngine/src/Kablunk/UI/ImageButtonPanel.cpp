#include "kablunkpch.h"
#include "Kablunk/UI/ImageButtonPanel.h"

#include "Kablunk/Renderer/backend/texture.h"
#include "Kablunk/Renderer/SceneRenderer.h"

namespace kb::ui
{

ImageButtonPanel::ImageButtonPanel(const glm::vec2& pos, const glm::vec2& size, arc<render::backend::texture_2d> image, uuid::uuid64 id /*= uuid::generate()*/)
	: Panel(pos, size, id)
{
	m_panel_style.image = image;
}

void ImageButtonPanel::on_left_mouse_down()
{
	KB_CORE_INFO("IMAGE BUTTON FCLIKC!");
}

}
