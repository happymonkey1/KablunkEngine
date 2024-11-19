#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_CONTEXT_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_CONTEXT_H

#include "Kablunk/renderer/backend/graphics_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_logical_device.h"
#include "kablunk/renderer/backend/vulkan/vulkan_swap_chain.h"

#include "kablunk/renderer/render_command.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct GLFWwindow;

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

// Owns the vulkan instance, devices (physical and logical), and swap chain.
// Handles vulkan validation layers in debug mode.
class vulkan_context final : public graphics_context {
public:
	vulkan_context(GLFWwindow* window_handle);
	~vulkan_context() override = default;

    vulkan_context(const vulkan_context&) = delete;
    vulkan_context(vulkan_context&&) = delete;

    auto operator=(const vulkan_context&) noexcept -> vulkan_context& = delete;
    auto operator=(vulkan_context&&) noexcept -> vulkan_context& = delete;

    static auto get() noexcept -> vulkan_context* { return s_context; }

    void init() noexcept override;

	void swap_buffers() override;

    void* get_window_handle() const noexcept override { return m_window_handle; }

    void set_window_handle(void* p_window_handle) noexcept override
    {
        m_window_handle = static_cast<GLFWwindow*>(p_window_handle);
    }

    void destroy() noexcept override;

    auto get_vk_instance() const noexcept -> VkInstance { return m_vk_instance; }

	const arc<vulkan_logical_device>& get_device() const noexcept { return m_device; }
    arc<vulkan_logical_device>& get_device() noexcept { return m_device; }

    // Retrieve a non-owning, immutable pointer to the swap chain
    const swap_chain* get_swap_chain() const noexcept override { return m_swap_chain.get();  }

    // Retrieve a non-owning, mutable pointer to the swap chain
	swap_chain* get_swap_chain() noexcept override { return m_swap_chain.get(); }

    // Retrieve a non-owning, immutable pointer to the swap chain
    auto get_vulkan_swap_chain() const noexcept -> const vulkan_swap_chain* { return m_swap_chain.get(); }

    // Retrieve a non-owning, mutable pointer to the swap chain
    auto get_vulkan_swap_chain() noexcept -> vulkan_swap_chain* { return m_swap_chain.get(); }

private:
	void create_instance();
	bool check_validation_layer_support() const;
	std::vector<const char*> get_required_extensions();

	// Debug callback for validation layer messages
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType,
		const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData
    )
	{
		if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT)
			KB_CORE_TRACE("[Vk Trace]: {0}", pCallbackData->pMessage);
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
			KB_CORE_INFO("[Vk Info]: {0}", pCallbackData->pMessage);
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
			KB_CORE_WARN("[Vk Warn]: {0}", pCallbackData->pMessage);
		else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
			KB_CORE_ERROR("[Vk Error]: {0}", pCallbackData->pMessage);
		else
		{
			KB_CORE_WARN("Unknown VkMessageType!");
			KB_CORE_ERROR("[Vk Unknown]: {0}", pCallbackData->pMessage);
		}

		return VK_FALSE;
	}

	// Load Debug Extension Proxy
	VkResult CreateDebugUtilsMessengerExtension(
        VkInstance instance,
        const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
        const VkAllocationCallbacks* pAllocator,
        VkDebugUtilsMessengerEXT* pDebugMessenger
    ) 
	{
		auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
		if (func != nullptr)
			return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
		else
			return VK_ERROR_EXTENSION_NOT_PRESENT;
	}

	// Destroy Debug Extension Proxy 
	void DestroyDebugUtilsMessengerEXT(
        VkInstance instance,
        VkDebugUtilsMessengerEXT debugMessenger,
        const VkAllocationCallbacks* pAllocator
    ) 
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
			func(instance, debugMessenger, pAllocator);
	}

	void SetupDebugMessageCallback();

private:
    inline static vulkan_context* s_context = nullptr;
    VkInstance m_vk_instance{};
	GLFWwindow* m_window_handle;

	arc<vulkan_physical_device> m_physical_device{};
	arc<vulkan_logical_device> m_device{};

	std::unique_ptr<vulkan_swap_chain> m_swap_chain;

	VkPipelineCache m_pipeline_cache;

	// Validation layers
	const std::vector<const char*> m_validation_layers;
	VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
#ifdef KB_DEBUG
	const bool m_enable_validation_layers = true;
#else
	const bool m_enable_validation_layers = false;
#endif

	friend class vulkan_logical_device;
};

} // start namespace kb::render::backend::vk

#endif
