#include "kablunkpch.h"


#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_allocator.h"

#include "kablunk/vendor/glfw/glfw.h"

#include <vector>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

vulkan_context::vulkan_context(GLFWwindow* window_handle)
	: m_window_handle{ window_handle }, m_pipeline_cache{ nullptr }, m_validation_layers{ "VK_LAYER_KHRONOS_validation" }
{
    KB_CORE_ASSERT(!s_context, "[vulkan_context]: Vulkan context is already initialized!");

    if (!s_context)
    {
        s_context = this;
    }

    init();
}

void vulkan_context::init() noexcept
{
	KB_CORE_INFO("Initializing Vulkan Context!");
    create_instance();

	SetupDebugMessageCallback();

	m_physical_device = arc<vulkan_physical_device>::Create(m_vk_instance);

	VkPhysicalDeviceFeatures enabled_features{};
	enabled_features.samplerAnisotropy = true;
	enabled_features.wideLines = true;
	enabled_features.fillModeNonSolid = true;
	enabled_features.pipelineStatisticsQuery = true;
	enabled_features.independentBlend = VK_TRUE;

	m_device = arc<vulkan_logical_device>::Create(
        m_vk_instance,
        m_physical_device,
        enabled_features
    );
	vulkan_allocator::init(get_device());

	m_swap_chain = std::make_unique<vulkan_swap_chain>();
	m_swap_chain->init(m_vk_instance, m_device);

	// Pipeline Cache
	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
	pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if (vkCreatePipelineCache(m_device->get_vk_device(), &pipelineCacheCreateInfo, nullptr, &m_pipeline_cache) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create pipeline cache!");
}

void vulkan_context::swap_buffers()
{
	m_swap_chain->present();
}

void vulkan_context::create_instance()
{
    KB_CORE_ASSERT(glfwInit(), "COULD NOT INITIALIZE GLFW");
    KB_CORE_ASSERT(glfwVulkanSupported(), "[vulkan_context]: GLFW can not load Vulkan!");

	KB_CORE_INFO("Creating Vulkan instance!");

	VkApplicationInfo app_info{};
	app_info.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	app_info.pApplicationName	= "KablunkEngine";
	app_info.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
	app_info.pEngineName		= "KablunkEngine";
	app_info.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
	app_info.apiVersion			= VK_API_VERSION_1_2;

	auto glfw_extensions = get_required_extensions();

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

	if (vkCreateInstance(&create_info, nullptr, &m_vk_instance) != VK_SUCCESS)
	{
		KB_CORE_ERROR("failed to create Vulkan instance!");
		return;
	}

	// Check for extension support
	uint32_t extension_count = 0;
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, nullptr);
	std::vector<VkExtensionProperties> extensions(extension_count);
	vkEnumerateInstanceExtensionProperties(nullptr, &extension_count, extensions.data());

	KB_CORE_INFO("Available Vulkan Extensions:");
	for (const auto& extension : extensions)
		KB_CORE_INFO("  {0}", extension.extensionName);

	if (m_enable_validation_layers && !check_validation_layer_support())
	{
		KB_CORE_ERROR("validation layers requested but not found!");
		return;
	}
}

bool vulkan_context::check_validation_layer_support() const
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

std::vector<const char*> vulkan_context::get_required_extensions()
{
    glfwInitVulkanLoader(vkGetInstanceProcAddr);
	uint32_t glfw_extension_count = 0;
	const char** glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);

	std::vector extensions(glfw_extensions, glfw_extensions + glfw_extension_count);
    

	if (m_enable_validation_layers)
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);

	return extensions;
}

void vulkan_context::SetupDebugMessageCallback()
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

	if (CreateDebugUtilsMessengerExtension(m_vk_instance, &create_info, nullptr, &m_debug_messenger) != VK_SUCCESS)
	{
		KB_CORE_ERROR("Failed to create Debug Messenger!");
		return;
	}
}

void vulkan_context::destroy() noexcept
{
    if (!m_vk_instance)
        return;

    KB_CORE_INFO("Destroying Vulkan instance");

    m_swap_chain->destroy();
    m_device->destroy();

    if (m_enable_validation_layers)
        DestroyDebugUtilsMessengerEXT(m_vk_instance, m_debug_messenger, nullptr);

    vkDestroyInstance(m_vk_instance, nullptr);
    m_vk_instance = nullptr;

    s_context = nullptr;

    KB_CORE_INFO("Finished destroying Vulkan instance");
}

} // end namespace kb::render::backend::vk
