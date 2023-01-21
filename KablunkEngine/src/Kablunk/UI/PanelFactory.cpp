#include "kablunkpch.h"
#include "Kablunk/UI/PanelFactory.h"
#include "Kablunk/Renderer/Texture.h"
#include "Kablunk/Renderer/RenderCommand2D.h"


#include "Kablunk/UI/Panel.h"
#include "Kablunk/UI/ImageButtonPanel.h"

namespace Kablunk::ui
{

	IPanel* PanelFactory::create_panel(panel_type_t panel_type, const panel_create_info_t& panel_create_info)
	{
		IPanel* panel = nullptr;
		switch (panel_type)
		{
			case panel_type_t::Blank:		panel = new Panel{ panel_create_info.position, panel_create_info.size }; break;
			case panel_type_t::ImageButton:	
			{
				ref<Texture2D> image = panel_create_info.image ? panel_create_info.image : render2d::get_white_texture();
				panel = new ImageButtonPanel{ panel_create_info.position, panel_create_info.size, image };
				break;
			}
			case panel_type_t::NONE:		KB_CORE_ASSERT(false, "invalid panel type NONE!"); return nullptr;
			default:						KB_CORE_ASSERT(false, "unhandled panel type!"); return nullptr;
		}

		panel->set_panel_type(panel_type);
		return panel;
	}

	IPanel* PanelFactory::copy_panel(IPanel* panel)
	{
		if (!panel)
			return nullptr;

		IPanel* new_panel = create_panel(panel->get_panel_type(), {});
		new_panel->set_position(panel->get_position());
		new_panel->set_size(panel->get_size());
		new_panel->set_style(panel->get_panel_style());

		return new_panel;
	}

	const char* panel_type_to_c_str(panel_type_t panel_type)
	{
		switch (panel_type)
		{
			case panel_type_t::Blank:			return "blank panel";
			case panel_type_t::ImageButton:		return "image button panel";
			case panel_type_t::NONE:			return "INV_PANEL_TYPE";
			default:							KB_CORE_ASSERT(false, "unhandled panel type!"); return "UNHANDLED_PANEL_TYPE";
		}
	}

}
