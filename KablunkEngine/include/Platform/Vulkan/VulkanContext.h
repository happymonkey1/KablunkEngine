#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_CONTEXT_H
#define KABLUNK_PLATFORM_VULKAN_CONTEXT_H

#include "Kablunk/Renderer/GraphicsContext.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

struct GLFWwindow;

namespace Kablunk
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> Graphics_family;

		bool IsComplete() {
			return Graphics_family.has_value();
		}
	};

	class VulkanContext : public GraphicsContext {
	public:
		VulkanContext(GLFWwindow* window_handle);

		void Init() override;
		void SwapBuffers() override;
		void Shutdown() override;
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
				KB_CORE_TRACE("[Vulkan Validation Layer]: {0}", pCallbackData->pMessage);
			else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT)
				KB_CORE_INFO("[Vulkan Validation Layer]: {0}", pCallbackData->pMessage);
			else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
				KB_CORE_WARN("[Vulkan Validation Layer]: {0}", pCallbackData->pMessage);
			else if (messageType & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT)
				KB_CORE_ERROR("[Vulkan Validation Layer]: {0}", pCallbackData->pMessage);
			else
			{
				KB_CORE_WARN("Unknown VkMessageType!");
				KB_CORE_INFO("[Vulkan Validation Layer]: {0}", pCallbackData->pMessage);
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

		// Physical Devices
		void PickPhysicalDevice();
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);

		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);

		// Logical Device
		void CreateLogicalDevice();
	private:
		GLFWwindow* m_window_handle;
		VkInstance m_instance;

		// Validation layers
		const std::vector<const char*> m_validation_layers;
		VkDebugUtilsMessengerEXT m_debug_messenger;
#ifdef KB_DEBUG
		const bool m_enable_validation_layers = true;
#else
		const bool m_enable_validation_layers = false;
#endif
	};
}

#endif
