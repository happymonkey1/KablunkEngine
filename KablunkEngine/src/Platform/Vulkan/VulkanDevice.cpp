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
			KB_CORE_ASSERT(false, "Vulkan found no suitable physical device!");

		m_queue_family_indices = FindQueueFamilies(m_device);

		uint32_t extension_count = 0;
		vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extension_count, nullptr);
		if (extension_count > 0)
		{
			std::vector<VkExtensionProperties> extensions(extension_count);
			if (vkEnumerateDeviceExtensionProperties(m_device, nullptr, &extension_count, extensions.data()) != VK_SUCCESS)
				KB_CORE_ASSERT(false, "Vulkan unable to find extensions!");

			KB_CORE_TRACE("Selected physical device has {0} extensions!", extension_count);
			for (const auto& extension : extensions)
				m_supported_extensions.push_back(extension.extensionName);
		}
	}

	void VulkanPhysicalDevice::FindPresentingIndices(VkSurfaceKHR surface)
	{
		// query if device has presenting support
		uint32_t i = 0;
		for (const auto& queue_family : m_queue_family_properties)
		{
			VkBool32 present_support = false;
			vkGetPhysicalDeviceSurfaceSupportKHR(m_device, i, surface, &present_support);
			
			if (present_support)
			{
				m_queue_family_indices.Present_family = i;
				break;
			}

			i++;
		}
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

		bool suitable = device_properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU && device_features.geometryShader && indices.HasGraphics();

		return suitable;
	}

	VulkanPhysicalDevice::~VulkanPhysicalDevice()
	{

	}

	VulkanDevice::VulkanDevice(const IntrusiveRef<VulkanPhysicalDevice>& physical_device, VkPhysicalDeviceFeatures enabled_features)
		: m_physical_device{ physical_device }, m_enabled_features{ enabled_features }
	{
		auto context = VulkanContext::Get();
		float queue_priority = 1.0f;
		std::vector<const char*> device_extensions;

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
		Destroy();
	}

	void VulkanDevice::Destroy()
	{
		vkDestroyCommandPool(m_device, m_command_pool, nullptr);
		
		vkDeviceWaitIdle(m_device);
		vkDestroyDevice(m_device, nullptr);
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

		VkFence fence;
		if (vkCreateFence(m_device, &fence_create_info, nullptr, &fence) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to create fence");

		if (vkWaitForFences(m_device, 1, &fence, VK_TRUE, DEFAULT_FENCE_TIMEOUT) != VK_SUCCESS)
			KB_CORE_ERROR("Vulkan failed to wait for fence!");

		vkDestroyFence(m_device, fence, nullptr);
		vkFreeCommandBuffers(m_device, m_command_pool, 1, &command_buffer);
	}

	VkCommandBuffer VulkanDevice::CreateSecondaryCommandBuffer()
	{
		KB_CORE_ASSERT(false, "not implemented");
		return nullptr;
	}
}
