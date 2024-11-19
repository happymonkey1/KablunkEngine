#pragma once
#ifndef KABLUNK_UI_IMAGE_BUTTON_PANEL_H
#define KABLUNK_UI_IMAGE_BUTTON_PANEL_H

#include "Kablunk/UI/Panel.h"

// forward declaration
namespace kb::render::backend
{
class texture_2d;
}

namespace kb::ui
{

class ImageButtonPanel : public Panel
{
public:
	ImageButtonPanel() = default;
	ImageButtonPanel(const glm::vec2& pos, const glm::vec2& size, arc<render::backend::texture_2d> image, uuid::uuid64 id = uuid::generate());
	virtual ~ImageButtonPanel() { destroy(); }

	virtual void on_left_mouse_down() override;
};

}

#endif
