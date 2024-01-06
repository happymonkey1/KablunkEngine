#pragma once
#ifndef KABLUNK_UI_PANEL_FACTORY_H
#define KABLUNK_UI_PANEL_FACTORY_H

#include "Kablunk/UI/IPanel.h"
#include "Kablunk/Utilities/EnumIterator.hpp"

// forward declarations
namespace kb
{
	class Texture2D;
}

namespace kb::ui
{

	struct panel_create_info_t
	{
		glm::vec2 position = glm::vec2{ 0.0f };
		glm::vec2 size = glm::vec2{ 0.5f, 0.5f };
		ref<Texture2D> image = {};
	};

	class PanelFactory
	{
	public:
		static IPanel* create_panel(panel_type_t panel_type, const panel_create_info_t& panel_create_info);
		static IPanel* copy_panel(IPanel* panel);
	};

}

#endif
