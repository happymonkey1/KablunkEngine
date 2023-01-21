#pragma once
#ifndef KABLUNK_UI_IMAGE_BUTTON_PANEL_H
#define KABLUNK_UI_IMAGE_BUTTON_PANEL_H

#include "Kablunk/UI/Panel.h"

// forward declaration
namespace Kablunk
{
	class Texture2D;
}

namespace Kablunk::ui
{

	class ImageButtonPanel : public Panel
	{
	public:
		ImageButtonPanel() = default;
		ImageButtonPanel(const glm::vec2& pos, const glm::vec2& size, ref<Texture2D> image, uuid::uuid64 id = uuid::generate());
		virtual ~ImageButtonPanel() { destroy(); }

		virtual void on_left_mouse_down() override;
	};

}

#endif
