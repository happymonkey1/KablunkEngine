#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanDevice.h"
#include "Platform/Vulkan/VulkanContext.h"


namespace Kablunk
{

	VulkanPhysicalDevice::VulkanPhysicalDevice()
		: m_device{ VK_NULL_HANDLE }
	{
		auto vkInstance = VulkanContext::GetInstance();

		uint32_t device_count = 0;
		vkEnumeratePhysicalDevices(vkInstance, &device_count, nullptr);

		if (device_count == 0)
			KB_CORE_ASSERT(false, "Vulkan found no physical devices!");

		std::vector<VkPhysicalDevice> devices(device_count);
		vkEnumeratePhysicalDevices(vkInstance, &device_count, devices.data());

		for (const auto& device : devices)
		{
			if (IsPhysicalDeviceSuitable(device))
			{
				// #TODO select best device
				m_device = device;
				break;
			}
		}

		if (m_device == VK_NULL_HANDLE)
		{
			KB_CORE_ASSERT(false, "Vulkan found no suitable physical device!");
		}
		else
		{
			KB_CORE_INFO("Vulkan selected device {0}", (const char*)m_properties.deviceName);
		}
		

		m_queue_family_indices = FindQueueFamilies(m_device);

		auto supported_extensions = FindSupportedExtensions(m_device);
		std::vector<const char*> supported_extensions_named;
		supported_extensions_named.reserve(supported_extensions.size());
		for (const auto& extension : supported_extensions)
			supported_extensions_named.emplace_back(extension.extensionName);
		m_supported_extensions = supported_extensions_named;

		m_depth_format = FindDepthFormat();
	}

	void VulkanPhysicalDevice::FindPresentingIndices(VkSurfaceKHR surface)
	{
		// query if device has presenting support. Iterate to find which queues support present
		std::vector<VkBool32> supports_present(m_queue_family_properties.size());
		uint32_t i = 0;
		for (const auto& queue_family : m_queue_family_properties)
		{
			vkGetPhysicalDeviceSurfaceSupportKHR(m_device, i, surface, &supports_present[i]);

			i++;
		}

		uint32_t graphics_queue_index = UINT32_MAX;
		uint32_t present_queue_index = UINT32_MAX;
		for (uint32_t i = 0; i < m_queue_family_properties.size(); i++)
		{
			if ((m_queue_family_properties[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) != 0)
			{
				if (graphics_queue_index == UINT32_MAX)
				{
					graphics_queue_index = i;
				}

				if (supports_present[i] == VK_TRUE)
				{
					graphics_queue_index = i;
					present_queue_index = i;
					break;
				}
			}
		}

		if (present_queue_index == UINT32_MAX)
		{
			// If there's no queue that supports both present and graphics
			// try to find a separate present queue
			for (uint32_t i = 0; i < m_queue_family_properties.size(); ++i)
			{
				if (supports_present[i] == VK_TRUE)
				{
					present_queue_index = i;
					break;
				}
			}
		}

		KB_CORE_ASSERT(present_queue_index != UINT32_MAX, "no present queue found!");
		KB_CORE_ASSERT(graphics_queue_index != UINT32_MAX, "no graphics queue found!");

		m_queue_family_indices.Graphics_family = graphics_queue_index;
		m_queue_family_indices.Present_family = present_queue_index;
	}

	QueueFamilyIndices VulkanPhysicalDevice::FindQueueFamilies(VkPhysicalDevice device)
	{
		QueueFamilyIndices queue_family_indices;
		uint32_t queue_family_count = 0;
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, nullptr);

		m_queue_family_properties = std::vector<VkQueueFamilyProperties>(queue_family_count);
		vkGetPhysicalDeviceQueueFamilyProperties(device, &queue_family_count, m_queue_family_properties.data());

		int i = 0;
		for (const auto& queue_family : m_queue_family_properties)
		{
			if (queue_family.queueFlags & VK_QUEUE_GRAPHICS_BIT)
				queue_family_indices.Graphics_family = i;

			if (queue_family_indices.HasGraphics())
				break;

			i++;
		}

		return queue_family_indices;
	}

	bool VulkanPhysicalDevice::IsPhysicalDeviceSuitable(VkPhysicalDevice device)
	{
		auto indices = FindQueueFamilies(device);
		VkPhysicalDeviceProperties device_properties{};
		VkPhysicalDeviceFeatures device_features{};

		vkGetPhysicalDeviceProperties(device, &device_properties);
		vkGetPhysicalDeviceFeatures(device, &device_features);

		// #TODO score each device and pick best

		bool suitable = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader 
			&& indices.HasGraphics() && CheckDeviseExtensionSupport(device);

		if (suitable)
			m_properties = device_properties;

		return suitable;
	}
	
	std::vector<VkExtensionProperties> VulkanPhysicalDevice::FindSupportedExtensions(VkPhysicalDevice device)
	{
		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, nullptr);
		if (extension_count > 0)
		{
			std::vector<VkExtensionProperties> extensions(extension_count);
			if (vkEnumerateDeviceExtensionProperties(device, nullptr, &extension_count, extensions.data()) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan unable to find extensions!");

			KB_CORE_TRACE("Physical device has {0} extensions!", extension_count);

			return extensions;
		}

		KB_CORE_WARN("Physical device has 0 extensions!");
		return {};
	}

	VkFormat VulkanPhysicalDevice::FindDepthFormat()
	{
		std::vector<VkFormat> depth_formats = {
			VK_FORMAT_D32_SFLOAT_S8_UINT,
			VK_FORMAT_D32_SFLOAT,
			VK_FORMAT_D24_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM_S8_UINT,
			VK_FORMAT_D16_UNORM
		};

		for (auto& format : depth_formats)
		{
			VkFormatProperties format_props;
			vkGetPhysicalDeviceFormatProperties(m_device, format, &format_props);
			// Format must support depth stencil attachment for optimal tiling
			if (format_props.optimalTilingFeatures & VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
				return format;
		}

		return VK_FORMAT_UNDEFINED;
	}

	bool VulkanPhysicalDevice::CheckDeviseExtensionSupport(VkPhysicalDevice device)
	{
		auto supported_extensions = FindSupportedExtensions(device);
		std::vector<const char*> supported_extensions_named;
		supported_extensions_named.reserve(supported_extensions.size());
		for (const auto& extension : supported_extensions)
			supported_extensions_named.emplace_back(extension.extensionName);
		
		for (const auto& extension : m_required_extensions)
		{
			bool found = false;
			for (const auto& supported : supported_extensions_named)
			{
				if (strcmp(supported, extension) == 0)
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

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{

	}

	VulkanDevice::VulkanDevice(const IntrusiveRef<VulkanPhysicalDevice>& physical_device, VkPhysicalDeviceFeatures enabled_features)
		: m_physical_device{ physical_device }, m_enabled_features{ enabled_features }
	{
		auto context = VulkanContext::Get();
		float queue_priority = 1.0f;
		const auto& device_extensions = m_physical_device->GetRequiredExtensions();

		// #TODO move to VulkanPhysicalDevice
		VkDeviceQueueCreateInfo queue_create_info{};
		queue_create_info.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queue_create_info.queueFamilyIndex = m_physical_device->GetQueueFamilyIndices().Graphics_family.value();
		queue_create_info.queueCount = 1;
		queue_create_info.pQueuePriorities = &queue_priority;

		// setup logical device
		VkDeviceCreateInfo create_info{};
		create_info.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
		create_info.pQueueCreateInfos = &queue_create_info;
		create_info.queueCreateInfoCount = 1;
		create_info.pEnabledFeatures = &m_enabled_features;


		if (context->m_enable_validation_layers)
		{
			create_info.enabledLayerCount = static_cast<uint32_t>(context->m_validation_layers.size());
			create_info.ppEnabledLayerNames = context->m_validation_layers.data();
		}
		else
			create_info.enabledLayerCount = 0;

		if (device_extensions.size() > 0)
		{
			create_info.enabledExtensionCount = static_cast<uint32_t>(device_extensions.size());
			create_info.ppEnabledExtensionNames = device_extensions.data();
		}
		else
			create_info.enabledExtensionCount = 0;

		// create logical device
		if (vkCreateDevice(m_physical_device->GetVkDevice(), &create_info, nullptr, &m_device) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Failed to create logical device!");

		VkCommandPoolCreateInfo cmd_pool_info{};
		cmd_pool_info.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
		KB_CORE_ASSERT(m_physical_device->GetQueueFamilyIndices().HasGraphics(), "no complete queue family!");
		cmd_pool_info.queueFamilyIndex = m_physical_device->GetQueueFamilyIndices().Graphics_family.value();
		cmd_pool_info.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
		
		if (vkCreateCommandPool(m_device, &cmd_pool_info, nullptr, &m_command_pool) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create command pool!");

		vkGetDeviceQueue(m_device, m_physical_device->GetQueueFamilyIndices().Graphics_family.value(), 0, &m_graphics_queue);
	}

	VulkanDevice::~VulkanDevice()
	{
		if (!m_destroyed)
			Destroy();
	}

	void VulkanDevice::Destroy()
	{
		if (m_destroyed)
			return;

		vkDestroyCommandPool(m_device, m_command_pool, nullptr);
		
		vkDeviceWaitIdle(m_device);
		vkDestroyDevice(m_device, nullptr);

		m_destroyed = true;
	}

	VkCommandBuffer VulkanDevice::GetCommandBuffer(bool begin)
	{
		VkCommandBuffer cmd_buf;

		VkCommandBufferAllocateInfo cmd_buf_allocate_info{};
		cmd_buf_allocate_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buf_allocate_info.commandPool = m_command_pool;
		cmd_buf_allocate_info.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmd_buf_allocate_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(m_device, &cmd_buf_allocate_info, &cmd_buf) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate command buffer!");

		if (begin)
		{
			VkCommandBufferBeginInfo cmd_buffer_begin_info{};
			cmd_buffer_begin_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
			if (vkBeginCommandBuffer(cmd_buf, &cmd_buffer_begin_info) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan failed to begin cmd buffer!");
		}

		return cmd_buf;
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer command_buffer)
	{
		FlushCommandBuffer(command_buffer, m_graphics_queue);
	}

	void VulkanDevice::FlushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue)
	{
		constexpr uint64_t DEFAULT_FENCE_TIMEOUT = 100000000000u;

		KB_CORE_ASSERT(command_buffer != VK_NULL_HANDLE, "command buffer is null!");

		if (vkEndCommandBuffer(command_buffer) != VK_SUCCESS)
			KB_CORE_ERROR("failed to end command buffer!");

		VkSubmitInfo submit_info{};
		submit_info.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submit_info.commandBufferCount = 1;
		submit_info.pCommandBuffers = &command_buffer;

		// Create a fence to ensure the command buffer finishes
		VkFenceCreateInfo fence_create_info{};
		fence_create_info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fence_create_info.flags = 0;

		// Create Fence
		VkFence fence;
		if (vkCreateFence(m_device, &fence_create_info, nullptr, &fence) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to create fence");

		// Submit Queue
		if (vkQueueSubmit(queue, 1, &submit_info, fence) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to submit queue");

		// W
		KB_CORE_INFO("VulkanDevice waiting for fence");
		if (vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to wait for fence!");

		vkDestroyFence(m_device, fence, nullptr);
		vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
	}

	VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
	{
		VkCommandBuffer cmd_buffer;

		VkCommandBufferAllocateInfo cmd_buffer_alloc_info{};
		cmd_buffer_alloc_info.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmd_buffer_alloc_info.commandPool = m_command_pool;
		cmd_buffer_alloc_info.level = VK_COMMAND_BUFFER_LEVEL_SECONDARY;
		cmd_buffer_alloc_info.commandBufferCount = 1;

		if (vkAllocateCommandBuffers(m_device, &cmd_buffer_alloc_info, &cmd_buffer) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate secondary buffer!");
		return cmd_buffer;

	}
}
