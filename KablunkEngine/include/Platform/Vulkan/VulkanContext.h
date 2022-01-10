#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_CONTEXT_H
#define KABLUNK_PLATFORM_VULKAN_CONTEXT_H

#include "Kablunk/Renderer/GraphicsContext.h"
#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanSwapChain.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct GLFWwindow;

namespace Kablunk
{
	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* window_handle);

		void Init() override;
		void SwapBuffers() override;
		void Shutdown() override;

		IntrusiveRef<VulkanDevice> GetDevice() { return m_device; }

		static IntrusiveRef<VulkanContext> Get() { return s_context; }

		static VkInstance GetInstance() { return s_instance; }

		VulkanSwapChain& GetSwapchain() { return m_swap_chain; }
	private:
		void CreateInstance();
		bool CheckValidationLayerSupport();
		std::vector<const char*> GetRequiredExtensions();

		// Debug callback for validation layer messages
		static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
			VkDebugUtilsMessageTypeFlagsEXT messageType,
			const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
			void* pUserData)
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
				KB_CORE_INFO("[Vk Unknown]: {0}", pCallbackData->pMessage);
			}

			return VK_FALSE;
		}

		// Load Debug Extension Proxy
		VkResult CreateDebugUtilsMessengerExtension(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo, const VkAllocationCallbacks* pAllocator, VkDebugUtilsMessengerEXT* pDebugMessenger) 
		{
			auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
			if (func != nullptr)
				return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
			else
				return VK_ERROR_EXTENSION_NOT_PRESENT;
		}

		// Destroy Debug Extension Proxy 
		void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks* pAllocator) 
		{
			auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
			if (func != nullptr)
				func(instance, debugMessenger, pAllocator);
		}

		void SetupDebugMessageCallback();
	private:
		// #TODO move
		inline static IntrusiveRef<VulkanContext> s_context;
		inline static VkInstance s_instance;

		GLFWwindow* m_window_handle;

		IntrusiveRef<VulkanPhysicalDevice> m_physical_device = nullptr;
		IntrusiveRef<VulkanDevice> m_device = nullptr;

		VulkanSwapChain m_swap_chain;

		// Validation layers
		const std::vector<const char*> m_validation_layers;
		VkDebugUtilsMessengerEXT m_debug_messenger = nullptr;
#ifdef KB_DEBUG
		const bool m_enable_validation_layers = true;
#else
		const bool m_enable_validation_layers = false;
#endif

		friend class VulkanDevice;
	};
}

#endif
