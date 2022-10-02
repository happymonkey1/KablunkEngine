#include "Panels/AssetRegistryPanel.h"

#include <Kablunk/Imgui/ImGuiWrappers.h>

namespace Kablunk
{

	void AssetRegistryPanel::on_imgui_render()
	{
		if (!m_open)
			return;

		if (ImGui::Begin("Asset Registry", &m_open))
		{
			{
				ImGui::BeginChild("Registry Entries");

				const asset::AssetRegistry& asset_registry = Singleton<asset::AssetManager>::get().get_asset_registry();
				
				UI::BeginProperties();

				UI::PropertyReadOnlyUint64("Entry Count", asset_registry.size());
				ImGui::Separator();

				int id = 0;
				for (const auto& [asset_id, metadata] : asset_registry)
				{
					ImGui::PushID(id++);

					std::string asset_id_str = fmt::format("{}", metadata.id);
					std::string asset_filepath = metadata.filepath.string();
					std::string asset_type = asset::asset_type_to_string(metadata.type);

					UI::PropertyReadOnlyString("uuid", asset_id_str);
					UI::PropertyReadOnlyString("filepath", asset_filepath);
					UI::PropertyReadOnlyString("type", asset_type);
					ImGui::Separator();
				}

				UI::EndProperties();

				ImGui::EndChild();
			}

			ImGui::End();
		}
	}

}
