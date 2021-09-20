#ifndef KABLUNK_IMGUI_IMGUI_WRAPPERS_H
#define KABLUNK_IMGUI_IMGUI_WRAPPERS_H

#include "Kablunk/Core/Log.h"
#include "Kablunk/Renderer/Texture.h"

#include "imgui.h"
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#	define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "imgui_internal.h"

#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <string>

namespace Kablunk::UI
{

	static uint32_t s_UIContextID = 0;
	constexpr uint32_t MAX_CHARS = 256;

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

			// Account for null terminator
			size_t label_size = std::strlen(label) + 1;
			KB_CORE_ASSERT(label_size <= MAX_CHARS, "label too long!");
			char* id_buffer = new char[label_size + 2];
			id_buffer[0] = '#';
			id_buffer[1] = '#';
			strcpy_s(&id_buffer[3], label_size, label);

			ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);
			DrawUI(id_buffer);
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

			size_t label_size = std::strlen(label) + 1;
			KB_CORE_ASSERT(label_size <= MAX_CHARS, "label too long!");
			char* id_buffer = new char[label_size + 2];
			id_buffer[0] = '#';
			id_buffer[1] = '#';
			strcpy_s(&id_buffer[2], label_size, label);

			if (IsItemDisabled())
				ImGui::PushStyleVar(ImGuiStyleVar_Alpha, ImGui::GetStyle().Alpha * 0.5f);

			bool modified = DrawUI(id_buffer);

			if (IsItemDisabled())
				ImGui::PopStyleVar();

			ImGui::PopItemWidth();
			ImGui::NextColumn();

			return modified;
		}
	}

	static void PushID()
	{
		ImGui::PushID(s_UIContextID++);
	}

	static void PopID()
	{
		ImGui::PopID();
		s_UIContextID--;
	}

	static void BeginProperties()
	{
		PushID();
		// #TODO update to table api before columns are deprecated
		ImGui::BeginColumns("##properties", 2, ImGuiOldColumnFlags_NoResize | ImGuiOldColumnFlags_NoBorder);
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
		ImGui::SetCursorPosY(ImGui::GetCursorPosX() + distance);
	}

	static void ShiftCursorY(float distance)
	{
		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + distance);
	}

	void Image(const Ref<Texture2D>& image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, const ImVec4& tint_col = { 1, 1, 1, 1 }, const ImVec4& border_col = { 0, 0, 0, 0 });
	bool ImageButton(const Ref<Texture2D>& image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));

	// Use BeginProperties() before and EndProperties() after!
	static bool Property(const char* label, std::string& value)
	{
		KB_CORE_ASSERT(strlen(label) + 1 < MAX_CHARS, "string is too long!");
		return Internal::CreateProperty(label, [&value](char* id_buffer) -> bool
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
		Internal::CreateStaticProperty(label, [&value](char* id_buffer)
			{
				ImGui::InputText(id_buffer, (char*)value.c_str(), value.size(), ImGuiInputTextFlags_ReadOnly);
			});
	}

	// Use BeginProperties() before and EndProperties() after!
	static bool Property(const char* label, char* value, size_t length)
	{
		return Internal::CreateProperty(label, [&value, &length](char* id_buffer) -> bool
			{
				return ImGui::InputText(id_buffer, (char*)value, length);
			});
	}

	// Use BeginProperties() before and EndProperties() after!
	static void Property(const char* label, const char* value)
	{
		Internal::CreateStaticProperty(label, [&value](char* id_buffer)
			{
				ImGui::InputText(id_buffer, (char*)value, MAX_CHARS, ImGuiInputTextFlags_ReadOnly);
			});
	}

	static bool Property(const char* label, float& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::DragFloat(id_buffer, &value, delta, min, max);
			});
	}

	static bool Property(const char* label, int& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::DragInt(id_buffer, &value, delta, min, max);
			});
	}

	static bool Property(const char* label, glm::vec2& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::DragFloat2(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool Property(const char* label, glm::vec3& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::DragFloat3(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool Property(const char* label, bool* value)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::Checkbox(id_buffer, value);
			});
	}

	static bool Property(const char* label, glm::vec4& value, float delta = 0.1f, float min = 0.0f, float max = 0.0f)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::DragFloat4(id_buffer, glm::value_ptr(value), delta, min, max);
			});
	}

	static bool PropertyColorEdit3(const char* label, glm::vec3& value)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::ColorEdit3(id_buffer, glm::value_ptr(value));
			});
	}

	static bool PropertyColorEdit4(const char* label, glm::vec4& value)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				return ImGui::ColorEdit4(id_buffer, glm::value_ptr(value));
			});
	}

	template <typename EnumT, typename UnderlyingT = int32_t>
	static bool PropertyDropdown(const char* label, const char** options, int32_t option_count, EnumT selected)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				UnderlyingT selected_index = (UnderlyingT)selected;
				const char* current = options[selected_index];
				bool updated = false;

				if (ImGui::BeginCombo(label, current))
				{
					for (size_t i = 0; i < option_count; ++i)
					{
						const bool selected = current == options[i];
						if (ImGui::Selectable(options[i], selected))
						{
							current = options[i];
							selected = (EnumT)i;
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

	static bool PropertyDropdown(const char* label, const std::vector<std::string>& options, int32_t option_count, int32_t* selected_index)
	{
		return Internal::CreateProperty(label, [&](char* id_buffer)
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

	static bool PropertyImageButton(const char* label, Ref<Texture2D> image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1))
	{
		ShiftCursorY(size.y / 4.0f);
		return Internal::CreateProperty(label, [&](char* id_buffer)
			{
				//ShiftCursorY(-5.0f);
				return ImageButton(image, size, uv0, uv1, frame_padding, bg_col, tint_col);
			});
	}

	static bool Button(const char* label, const ImVec2& size = { 0, 0 })
	{
		bool pressed = ImGui::Button(label, size);
		ImGui::NextColumn();
		return pressed;
	}

	static void EndProperties()
	{
		ImGui::EndColumns();
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
}

#endif
