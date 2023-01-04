#include "kablunkpch.h"

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanAllocator.h"

#include <vector>

namespace Kablunk
{
	VulkanContext::VulkanContext(GLFWwindow* window_handle)
		: m_window_handle{ window_handle }, m_validation_layers{ "VK_LAYER_KHRONOS_validation" }, m_pipeline_cache{ nullptr }
	{
		s_context = IntrusiveRef<VulkanContext>(this);
	}

	void VulkanContext::Init()
	{
		KB_CORE_INFO("Initializing Vulkan Context!");
		CreateInstance();
		SetupDebugMessageCallback();
		
		
		m_physical_device = IntrusiveRef<VulkanPhysicalDevice>::Create();

		VkPhysicalDeviceFeatures enabled_features{};
		enabled_features.samplerAnisotropy = true;
		enabled_features.wideLines = true;
		enabled_features.fillModeNonSolid = true;
		enabled_features.pipelineStatisticsQuery = true;
		enabled_features.independentBlend = VK_TRUE;

		m_device = IntrusiveRef<VulkanDevice>::Create(m_physical_device, enabled_features);
		VulkanAllocator::Init(VulkanContext::Get()->GetDevice());

		m_swap_chain = VulkanSwapChain{};
		m_swap_chain.Init(s_instance, m_device);


		// Pipeline Cache
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		if (vkCreatePipelineCache(m_device->GetVkDevice(), &pipelineCacheCreateInfo, nullptr, &m_pipeline_cache) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create pipeline cache!");
	}

	void VulkanContext::SwapBuffers()
	{
		m_swap_chain.Present();
	}

	void VulkanContext::CreateInstance()
	{
		KB_CORE_INFO("Creating Vulkan instance!");

		VkApplicationInfo app_info{};
		app_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
		app_info.pApplicationName	= "KablunkEngine";
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
		
		if (vkCreateInstance(&create_info, nullptr, &s_instance) != VK_SUCCESS)
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
		create_info.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
		create_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
		create_info.pfnUserCallback = DebugCallback;
		create_info.pUserData = nullptr;

		if (CreateDebugUtilsMessengerExtension(s_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
		{
			KB_CORE_ERROR("Failed to create Debug Messenger!");
			return;
		}
	}

	void VulkanContext::Shutdown()
	{
		KB_CORE_INFO("Shutting down Vulkan instance");

		m_swap_chain.Destroy();

		// device needs to reference the vulkan context to be destroyed
		// device destruction is now destroyed with the window, before this context
		// is destroyed
		//m_device->Destroy();

		if (m_enable_validation_layers)
			DestroyDebugUtilsMessengerEXT(s_instance, m_debug_messenger, nullptr);

		vkDestroyInstance(s_instance, nullptr);
		s_instance = nullptr;
	}

}
