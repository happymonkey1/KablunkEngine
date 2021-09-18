#ifndef KABLUNK_IMGUI_IMGUI_WRAPPERS_H
#define KABLUNK_IMGUI_IMGUI_WRAPPERS_H

#include "imgui.h"
#include "Kablunk/Renderer/Texture.h"

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
			ImGui::Text(label);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			// Account for null terminator
			size_t label_size = std::strlen(label) + 1;
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
			ImGui::Text(label);
			ImGui::NextColumn();
			ImGui::PushItemWidth(-1);

			size_t label_size = std::strlen(label) + 1;
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

	// Use BeginProperties() before and EndProperties() after!
	static bool Property(const char* label, std::string& value)
	{
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

	void Image(const Ref<Texture2D>& image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, const ImVec4& tint_col = { 1, 1, 1, 1 }, const ImVec4& border_col = { 0, 0, 0, 0 });
	bool ImageButton(const Ref<Texture2D>& image, const ImVec2& size, const ImVec2& uv0 = { 0, 0 }, const ImVec2& uv1 = { 1, 1 }, int frame_padding = -1, const ImVec4& bg_col = ImVec4(0, 0, 0, 0), const ImVec4& tint_col = ImVec4(1, 1, 1, 1));
	
}

#endif
