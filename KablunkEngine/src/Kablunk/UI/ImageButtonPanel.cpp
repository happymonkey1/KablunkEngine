#include "kablunkpch.h"
#include "Kablunk/UI/ImageButtonPanel.h"

#include "Kablunk/Renderer/RenderCommand2D.h"

#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/SceneRenderer.h"

namespace Kablunk::ui
{

	ImageButtonPanel::ImageButtonPanel(const glm::vec2& pos, const glm::vec2& size, ref<Texture2D> image, uuid::uuid64 id /*= uuid::generate()*/)
		: Panel(pos, size, id)
	{
		m_panel_style.image = image;
	}

	void ImageButtonPanel::on_left_mouse_down()
	{
		KB_CORE_INFO("IMAGE BUTTON FCLIKC!");
	}


}
