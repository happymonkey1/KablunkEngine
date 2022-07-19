#ifndef KABLUNK_IMGUI_IMGUI_WRAPPERS_H
#define KABLUNK_IMGUI_IMGUI_WRAPPERS_H

#include "Kablunk/Core/Logger.h"
#include "Kablunk/Renderer/Texture.h"

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#	define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>
#include <string_view>

namespace Kablunk::UI
{
	constexpr uint32_t MAX_CHARS = 256;
	static char s_id_buffer[MAX_CHARS];
	static uint32_t s_ui_context_id = 0;

	static const char* GenerateID()
	{
		s_id_buffer[0] = '#';
		s_id_buffer[1] = '#';
		memset(s_id_buffer + 2, 0, 14);
		sprintf_s(s_id_buffer + 2, 14, "%o", s_ui_context_id++);

		return &s_id_buffer[0];
	}

	static const char* GenerateLabelID(std::string_view label)
	{
		*fmt::format_to_n(s_id_buffer, std::size(s_id_buffer), "{}##{}", label, s_ui_context_id++).out = 0;
		return s_id_buffer;
	}

	namespace Internal
	{
		template <typename FuncT>
		static void CreateStaticProperty(const char* label, FuncT DrawUI)
		{
			ShiftCursorY(3.0f);

			ImGui::Text(label);

			ShiftCursorY(-3.0f);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);
			
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			DrawUI(GenerateID());
			ImGui::PopStyleVar();

			ImGui::PopItemWidth();
			ImGui::NextColumn();
		}

		template <typename FuncT>
		static bool CreateProperty(const char* label, FuncT DrawUI)
		{
			ShiftCursorY(3.0f);

			ImGui::Text(label);

			ShiftCursorY(-3.0f);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			if (IsItemDisabled())
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

			bool modified = DrawUI(GenerateID());

			if (IsItemDisabled())
				ImGui::PopStyleVar();

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return modified;
		}
	}

	static void PushID()
	{
		ImGui::PushID(s_ui_context_id++);
		s_ui_context_id = 0;
	}

	static void PopID()
	{
		ImGui::PopID();
		s_ui_context_id--;
	}

	static void BeginProperties()
	{
		PushID();
		// #TODO update to table api before columns are deprecated
		ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, { 8.0f, 8.0f });
		ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, { 4.0f, 4.0f });
		ImGui::Columns(2);
	}

	static void PushItemDisabled()
	{
		ImGui::PushItemFlag(ImGuiItemFlags_Disabled, true);
	}

	static bool IsItemDisabled()
	{
		return ImGui::GetItemFlags() & ImGuiItemFlags_Disabled;
	}

	static void PopItemDisabled()
	{
		ImGui::PopItemFlag();
	}

	static bool IsMouseDownOnDockedWindow(ImGuiMouseButton mouse = 0, ImGuiHoveredFlags hovered_flags = 0)
	{
		return ImGui::IsMouseDown(mouse) && ImGui::IsWindowHovered(hovered_flags);
	}

	static bool IsMouseDownOnItem(ImGuiMouseButton mouse = 0, ImGuiHoveredFlags flags = 0)
	{
		return ImGui::IsMouseDown(mouse) && ImGui::IsItemHovered(flags);
	}

	static void ShiftCursor(ImVec2 distance)
	{
		ImGui::SetCursorPos(ImGui::GetCursorPos() + distance);
	}

	static void ShiftCursorX(float distance)
	{
		ImGui::SetCursorPosX(ImGui::GetCursorPosX() + distance);
	}

	static void ShiftCursorY(float distance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
	}

	ImTextureID GetTextureID(IntrusiveRef<Texture2D> texture);
	void Image(const IntrusiveRef<Image2D>& image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, const ImVec4& tint_col = { 1, 1, 1, 1 }, const ImVec4& border_col = { 0, 0, 0, 0 });
	void Image(const IntrusiveRef<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, const ImVec4& tint_col = { 1, 1, 1, 1 }, const ImVec4& border_col = { 0, 0, 0, 0 });
	bool ImageButton(const IntrusiveRef<Texture2D>& texture, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

	// Use BeginProperties() before and EndProperties() after!
	static bool Property(const char* label, std::string& value)
	{
		KB_CORE_ASSERT(strlen(label) + 1 < MAX_CHARS, "string is too long!");
		return Internal::CreateProperty(label, [&value](const char* id_buffer) -> bool
			{
				char buffer[MAX_CHARS];
				strcpy_s(buffer, value.c_str());

				bool modified = false;
				if (ImGui::InputText(id_buffer, buffer, MAX_CHARS))
				{
					value = buffer;
					modified = true;
				}

				return modified;
			});
	}

	// Use BeginProperties() before and EndProperties() after!
	static void Property(const char* label, const std::string& value)
	{
		Internal::CreateStaticProperty(label, [&value](const char* id_buffer)
			{
				ImGui::InputText(id_buffer, (char*)value.c_str(), value.size(), ImGuiInputTextFlags_ReadOnly);
			});
	}

	static void PropertyWithHint(const char* label, const std::string& hint, std::string& value)
	{
		Internal::CreateStaticProperty(label, [&](const char* id_buffer)
			{
				ImGui::InputTextWithHint(id_buffer, hint.c_str(), (char*)value.c_str(), value.size(), ImGuiInputTextFlags_ReadOnly);
			});
	}

	static void PropertyReadOnlyString(const char* label, const std::string& value)
	{
		Property(label, value);
	}

	static void PropertyReadOnlyStringWithHint(const char* label, const std::string& hint, std::string& value)
	{
		PropertyWithHint(label, hint, value);
	}

	// Use BeginProperties() before and EndProperties() after!
	static bool Property(const char* label, char* value, size_t length)
	{
		return Internal::CreateProperty(label, [&value, &length](const char* id_buffer) -> bool
			{
				return ImGui::InputText(id_buffer, (char*)value, length);
			});
	}

	static bool PropertyWithHint(const char* label, const char* hint, char* value, size_t length)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer) -> bool
			{
				return ImGui::InputTextWithHint(id_buffer, hint, (char*)value, length);
			});
	}

	// Use BeginProperties() before and EndProperties() after!
	static void Property(const char* label, const char* value)
	{
		Internal::CreateStaticProperty(label, [&value](const char* id_buffer)
			{
				ImGui::InputText(id_buffer, (char*)value, MAX_CHARS, ImGuiInputTextFlags_ReadOnly);
			});
	}

	static void PropertyWithHint(const char* label, const char* hint, const char* value)
	{
		Internal::CreateStaticProperty(label, [&](const char* id_buffer)
			{
				ImGui::InputTextWithHint(id_buffer, hint, (char*)value, MAX_CHARS, ImGuiInputTextFlags_ReadOnly);
			});
	}

	static void PropertyReadOnlyChars(const char* label, const char* value)
	{
		Property(label, value);
	}

	static void PropertyReadOnlyCharsWithHint(const char* label, const char* hint, const char* value)
	{
		PropertyWithHint(label, hint, value);
	}

	static bool Property(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::DragFloat(id_buffer, &value, delta, min, max);
			});
	}

	static void PropertyReadOnlyFloat(const char* label, const float& value)
	{
		Property(label, std::to_string(value));
	}

	static void PropertyReadOnlyDouble(const char* label, const double& value)
	{
		Property(label, std::to_string(value));
	}

	static bool Property(const char* label, int& value, float delta = 0.1f, int min = 0.0f, int max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::DragInt(id_buffer, &value, delta, min, max);
			});
	}

	static bool Property(const char* label, uint32_t& value, float delta = 0.1f, int min = 0.0f, int max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				int casted_int = static_cast<int>(value);
				return ImGui::DragInt(id_buffer, &casted_int, delta, min, max);
			});
	}

	static void PropertyReadOnlyUint32(const char* label, const uint32_t& value)
	{
		Property(label, std::to_string(value));
	}

	static void PropertyReadOnlyUint64(const char* label, const uint64_t& value)
	{
		Property(label, std::to_string(value));
	}

	static void PropertyReadOnlyVec3(const char* label, const glm::vec3& value)
	{
		Property(label, fmt::format("{:.3f}, {:.3f}, {:.3f}", value.x, value.y, value.z));
	}

	static bool Property(const char* label, glm::vec2& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::DragFloat2(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool Property(const char* label, glm::vec3& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::DragFloat3(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool Property(const char* label, bool* value)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::Checkbox(id_buffer, value);
			});
	}

	static bool Property(const char* label, glm::vec4& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::DragFloat4(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool PropertyColorEdit3(const char* label, glm::vec3& value)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::ColorEdit3(id_buffer, glm::value_ptr(value));
			});
	}

	static bool PropertyColorEdit4(const char* label, glm::vec4& value)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				return ImGui::ColorEdit4(id_buffer, glm::value_ptr(value));
			});
	}

	template <typename EnumT, typename UnderlyingT = int32_t>
	static bool PropertyDropdown(const char* label, const char** options, int32_t option_count, EnumT& selected)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				UnderlyingT selected_index = (UnderlyingT)selected;
				const char* current = options[selected_index];
				bool updated = false;

				if (ImGui::BeginCombo(label, current))
				{
					for (size_t i = 0; i < option_count; ++i)
					{
						const bool is_selected = current == options[i];
						if (ImGui::Selectable(options[i], is_selected))
						{
							current = options[i];
							selected = (EnumT)i;
							updated = true;
						}

						if (is_selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				return updated;
			});
	}

	static bool PropertyDropdown(const char* label, const std::vector<std::string>& options, int32_t option_count, int32_t* selected_index)
	{
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				const char* current = options[*selected_index].c_str();
				bool updated = false;

				if (ImGui::BeginCombo(label, current))
				{
					for (size_t i = 0; i < option_count; ++i)
					{
						const bool selected = current == options[i];
						if (ImGui::Selectable(options[i].c_str(), selected))
						{
							current = options[i].c_str();
							*selected_index = i;
							updated = true;
						}

						if (selected)
							ImGui::SetItemDefaultFocus();
					}

					ImGui::EndCombo();
				}

				return updated;
			});
	}

	static bool PropertyImageButton(const char* label, IntrusiveRef<Texture2D> image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
	{
		ShiftCursorY(size.y / 4.0f);
		return Internal::CreateProperty(label, [&](const char* id_buffer)
			{
				//ShiftCursorY(-5.0f);
				return ImageButton(image, size, uv0, uv1, frame_padding, bg_col, tint_col);
			});
	}

	static bool Button(const char* label, const ImVec2& size = { 0, 0 }, bool next_column = true)
	{
		bool pressed = ImGui::Button(label, size);
		if (next_column)
			ImGui::NextColumn();
		return pressed;
	}

	static bool PropertyFolderPathWithButton(const char* label, const char* path_buffer, size_t buffer_size)
	{
		bool pressed = false;

		ImGui::EndColumns();
		if (ImGui::BeginTable("##folder_path_with_button", 3))
		{
			const auto& style = ImGui::GetStyle();
			auto button_size = ImGui::CalcTextSize("...");
			auto label_width = ImGui::GetWindowContentRegionWidth() / 2.0f - style.ColumnsMinSpacing - 1;
			ImGui::TableSetupColumn("##label", ImGuiTableColumnFlags_NoHeaderLabel);
			ImGui::TableSetupColumn("##path_preview", ImGuiTableColumnFlags_NoHeaderLabel);
			ImGui::TableSetupColumn("##button", ImGuiTableColumnFlags_NoHeaderLabel);
			ImGui::TableHeadersRow();
			ImGui::TableNextColumn();

			ShiftCursorY(3.0f);
			ImGui::Text(label);
			ShiftCursorY(-3.0f);

			ImGui::TableNextColumn();

			ImGui::PushItemWidth(-1);
			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

			ImGui::InputText("##path_preview", (char*)path_buffer, buffer_size, ImGuiInputTextFlags_ReadOnly);

			ImGui::PopStyleVar();
			ImGui::PopItemWidth();

			ImGui::TableNextColumn();

			pressed = Button("...", button_size, false);

			ImGui::EndTable();
		}

		// #TODO remove when switching main properties layout to tables API
		ImGui::BeginColumns("##properties", 2, ImGuiOldColumnFlags_NoResize | ImGuiOldColumnFlags_NoBorder);

		return pressed;
	}

	static void EndProperties()
	{
		ImGui::Columns(1);
		ImGui::PopStyleVar(2);
		UI::ShiftCursorY(18.0f);
		PopID();
	}

	static bool BeginTreeNode(const char* label, bool default_open = true)
	{
		auto flags = ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_FramePadding;
		if (default_open)
			flags |= ImGuiTreeNodeFlags_DefaultOpen;

		return ImGui::TreeNodeEx(label, flags);
	}

	static void EndTreeNode()
	{
		ImGui::TreePop();
	}

	
	// Button Images
	static void DrawButtonImage(const IntrusiveRef<Texture2D>& imageNormal, const IntrusiveRef<Texture2D>& imageHovered, const IntrusiveRef<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		auto* drawList = ImGui::GetWindowDrawList();
		if (ImGui::IsItemActive())
			drawList->AddImage(GetTextureID(imagePressed), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintPressed);
		else if (ImGui::IsItemHovered())
			drawList->AddImage(GetTextureID(imageHovered), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintHovered);
		else
			drawList->AddImage(GetTextureID(imageNormal), rectMin, rectMax, ImVec2(0, 0), ImVec2(1, 1), tintNormal);
	};

	static void DrawButtonImage(const IntrusiveRef<Texture2D>& imageNormal, const IntrusiveRef<Texture2D>& imageHovered, const IntrusiveRef<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};

	static void DrawButtonImage(const IntrusiveRef<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImVec2 rectMin, ImVec2 rectMax)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectMin, rectMax);
	};

	static void DrawButtonImage(const IntrusiveRef<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed,
		ImRect rectangle)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, rectangle.Min, rectangle.Max);
	};


	static void DrawButtonImage(const IntrusiveRef<Texture2D>& imageNormal, const IntrusiveRef<Texture2D>& imageHovered, const IntrusiveRef<Texture2D>& imagePressed,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(imageNormal, imageHovered, imagePressed, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};

	static void DrawButtonImage(const IntrusiveRef<Texture2D>& image,
		ImU32 tintNormal, ImU32 tintHovered, ImU32 tintPressed)
	{
		DrawButtonImage(image, image, image, tintNormal, tintHovered, tintPressed, ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	};


	// Rectangle
	static inline ImRect GetItemRect()
	{
		return ImRect(ImGui::GetItemRectMin(), ImGui::GetItemRectMax());
	}

	static inline ImRect RectExpanded(const ImRect& rect, float x, float y)
	{
		ImRect result = rect;
		result.Min.x -= x;
		result.Min.y -= y;
		result.Max.x += x;
		result.Max.y += y;
		return result;
	}

	// Color
	static ImU32 ColorWithMultipliedValue(const ImColor& color, float multiplier)
	{
		const ImVec4& colRow = color.Value;
		float hue, sat, val;
		ImGui::ColorConvertRGBtoHSV(colRow.x, colRow.y, colRow.z, hue, sat, val);
		return ImColor::HSV(hue, sat, std::min(val * multiplier, 1.0f));
	}

}

#endif
