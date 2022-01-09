#include "kablunkpch.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Platform/Vulkan/VulkanContext.h"

#include <vector>

namespace Kablunk
{
	VulkanContext::VulkanContext(GLFWwindow* window_handle)
		: m_window_handle{ window_handle }, m_instance{}, m_validation_layers{ "VK_LAYER_KHRONOS_validation" }
	{
		
	}

	void VulkanContext::Init()
	{
		CreateInstance();
		SetupDebugMessageCallback();
	}

	void VulkanContext::SwapBuffers()
	{

	}

	void VulkanContext::CreateInstance()
	{
		KB_CORE_INFO("Creating Vulkan instance!");

		VkApplicationInfo app_info{};
		app_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName	= "FIXME";
		app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
		app_info.pEngineName		= "KablunkEngine";
		app_info.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
		app_info.apiVersion			= VK_API_VERSION_1_2;

		
		auto glfw_extensions = GetRequiredExtensions();

		VkInstanceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
		create_info.pApplicationInfo = &app_info;
		create_info.enabledExtensionCount = static_cast<uint32_t>(glfw_extensions.size());
		create_info.ppEnabledExtensionNames = glfw_extensions.data();
		if (m_enable_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(m_validation_layers.size());
			create_info.ppEnabledLayerNames = m_validation_layers.data();
		}
		else
			create_info.enabledLayerCount = 0;
		
		if (vkCreateInstance(&create_info, nullptr, &m_instance) != VK_SUCCESS)
		{
			KB_CORE_ERROR("failed to create Vulkan instance!");
			return;
		}

		// Check for extension support
		uint32_t extension_count = 0;
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
		std::vector<VkExtensionProperties> extensions(extension_count);
		vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

		KB_CORE_TRACE("Available Vulkan Extensions:");
		for (const auto& extension : extensions)
			KB_CORE_TRACE("  {0}", extension.extensionName);

		// #TODO validate that required extensions by Vulkan are present in GLFW

		if (m_enable_validation_layers && !CheckValidationLayerSupport())
		{
			KB_CORE_ERROR("validation layers requested but not found!");
			return;
		}
	}

	bool VulkanContext::CheckValidationLayerSupport()
	{
		uint32_t layer_count;
		vkEnumerateInstanceLayerProperties(&layer_count, nullptr);

		std::vector<VkLayerProperties> available_layers(layer_count);
		vkEnumerateInstanceLayerProperties(&layer_count, available_layers.data());

		for (const char* layer_name : m_validation_layers)
		{
			bool found = false;

			for (const auto& layer_prop : available_layers)
			{
				if (strcmp(layer_name, layer_prop.layerName) == 0)
				{
					found = true;
					break;
				}
			}

			if (!found)
				return false;
		}

		return true;
	}

	std::vector<const char*> VulkanContext::GetRequiredExtensions()
	{
		uint32_t glfw_extension_count = 0;
		const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

		std::vector<const char*> extensions(glfw_extensions, glfw_extensions + glfw_extension_count);

		if (m_enable_validation_layers)
			extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

		return extensions;
	}

	void VulkanContext::SetupDebugMessageCallback()
	{
		KB_CORE_INFO("Setting up Vulkan validation layer(s)");
		if (!m_enable_validation_layers)
			return;

		VkDebugUtilsMessengerCreateInfoEXT create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = DebugCallback;
		create_info.pUserData = nullptr;

		if (CreateDebugUtilsMessengerExtension(m_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
		{
			KB_CORE_ERROR("Failed to create Debug Messenger!");
			return;
		}
	}

	void VulkanContext::PickPhysicalDevice()
	{
		VkPhysicalDevice physical_device = VK_NULL_HANDLE;

		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(m_instance, &device_count, nullptr);

		if (device_count == 0)
		{
			KB_CORE_ERROR("Vulkan found no physical devices!");
			return;
		}

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(m_instance, &device_count, devices.data());

		for (const auto& device : devices)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				// #TODO select best device
				physical_device = device;
				break;
			}
		}

		if (physical_device == VK_NULL_HANDLE)
		{
			KB_CORE_ERROR("Vulkan found no suitable physical device!");
			return;
		}
	}

	bool VulkanContext::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
	{
		VkPhysicalDeviceProperties device_properties{};
		VkPhysicalDeviceFeatures device_features{};
		
		vkGetPhysicalDeviceProperties(device, &device_properties);
		vkGetPhysicalDeviceFeatures(device, &device_features);

		// #TODO score each device and pick best

		auto indices = FindQueueFamilies(device);

		bool suitable = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader && indices.IsComplete();

		return suitable;
	}

	QueueFamilyIndices VulkanContext::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices indices;

		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		std::vector<VkQueueFamilyProperties> queue_families(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, queue_families.data());

		int i = 0;
		for (const auto& queue_family : queue_families)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				indices.Graphics_family = i;

			i++;
		}

		return indices;
	}

	void VulkanContext::CreateLogicalDevice()
	{
		auto indices = FindQueueFamilies()
	}

	void VulkanContext::Shutdown()
	{
		KB_CORE_INFO("Shutting down Vulkan instance");

		if (m_enable_validation_layers)
			DestroyDebugUtilsMessengerEXT(m_instance, m_debug_messenger, nullptr);

		vkDestroyInstance(m_instance, nullptr);
	}

}
