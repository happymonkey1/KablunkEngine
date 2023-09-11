#pragma once

#include "Kablunk/Core/Core.h"
#include "Kablunk/Core/Singleton.h"


#include <imgui.h>
#include <imgui_internal.h>

#ifndef KABLUNK_IMGUI_IMGUI_GLOBAL_CONTEXT
#define KABLUNK_IMGUI_IMGUI_GLOBAL_CONTEXT

namespace kb
{
	// #TODO should we forward declare ImGuiContext so we don't have to include headers?


	class ImGuiGlobalContext
	{
	public:
		void init() {};
		void shutdown() { m_imgui_context = nullptr; }

		// Set internal pointer to the current ImGui context
		void set_context(ImGuiContext* context) { KB_CORE_ASSERT(!m_imgui_context, "context already set!"); m_imgui_context = context; }
		
		// Return a pointer to the ImGui context
		ImGuiContext* get_context() const { KB_CORE_ASSERT(m_imgui_context, "context not set!"); return m_imgui_context; }

		// Get singleton instance
		SINGLETON_GET_FUNC(ImGuiGlobalContext)
	private:
		// pointer to the current ImGui context; must be set manually by engine
		ImGuiContext* m_imgui_context = nullptr;
	};

}

#endif
