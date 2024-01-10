#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_DEVICE_H
#define KABLUNK_PLATFORM_VULKAN_DEVICE_H

#include "Kablunk/Core/Core.h"

#include "Platform/Vulkan/vulkan_command_pool.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>
#include <map>
#include <thread>

namespace kb
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> Graphics_family;
		std::optional<uint32_t> Present_family;
        std::optional<uint32_t> m_compute_family;
        std::optional<uint32_t> m_transfer_family;

		bool HasGraphics() const { return Graphics_family.has_value(); }
		bool IsComplete() const { return Graphics_family.has_value() && Present_family.has_value() && m_compute_family.has_value() && m_transfer_family.has_value(); }
        bool has_compute() const { return m_compute_family.has_value(); }
        bool has_trasfer() const { return m_transfer_family.has_value(); }
	};

	// Physical Device
	class VulkanPhysicalDevice : public RefCounted
	{
	public:
		VulkanPhysicalDevice();
		~VulkanPhysicalDevice();

		VkPhysicalDevice GetVkDevice() { return m_device; }
		const QueueFamilyIndices& GetQueueFamilyIndices() const { return m_queue_family_indices; }

		const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
		const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }
		const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }
		const std::vector<const char*>& GetRequiredExtensions() const { return m_required_extensions; }
		VkFormat GetDepthFormat() const { return m_depth_format; }
	private:
		void FindPresentingIndices(VkSurfaceKHR surface);
		QueueFamilyIndices FindQueueFamilies(VkPhysicalDevice device);
		bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
		bool CheckDeviseExtensionSupport(VkPhysicalDevice device);
		std::vector<VkExtensionProperties> FindSupportedExtensions(VkPhysicalDevice device);
		VkFormat FindDepthFormat();
        void CreateQueueInfos();
	private:
		VkPhysicalDevice m_device = nullptr;

		VkPhysicalDeviceProperties m_properties;
		VkPhysicalDeviceFeatures m_features;

		VkFormat m_depth_format = VK_FORMAT_UNDEFINED;

		std::vector<VkQueueFamilyProperties> m_queue_family_properties;
		std::vector<VkDeviceQueueCreateInfo> m_queue_create_infos;

		std::vector<const char*> m_supported_extensions;
		std::vector<const char*> m_required_extensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };

		QueueFamilyIndices m_queue_family_indices;

		friend class VulkanSwapChain;
        friend class VulkanDevice;
	};

	// Logical Device
	class VulkanDevice : public RefCounted 
	{
	public:
		VulkanDevice(const ref<VulkanPhysicalDevice>& physical_device, VkPhysicalDeviceFeatures enabled_features);
		~VulkanDevice();

		void Destroy();

		VkQueue GetGraphicsQueue() { return m_vk_graphics_queue; }
        VkQueue get_vk_compute_queue() { return m_vk_compute_queue; }

		VkCommandBuffer GetCommandBuffer(bool begin, bool p_compute = false);
		void FlushCommandBuffer(VkCommandBuffer command_buffer);
		void FlushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue, kb::vk::command_buffer_type_t p_command_buffer_type);

		VkCommandBuffer CreateSecondaryCommandBuffer();

		ref<VulkanPhysicalDevice> GetPhysicalDevice() { return m_physical_device; }
		VkPhysicalDevice GetVkPhysicalDevice() { return m_physical_device->GetVkDevice(); }
		VkDevice GetVkDevice() { return m_vk_device; }

    private:
        ref<kb::vk::command_pool> get_thread_local_command_pool();
        ref<kb::vk::command_pool> get_or_create_thread_local_command_pool();

	private:
		VkDevice m_vk_device;
		ref<VulkanPhysicalDevice> m_physical_device;
		VkPhysicalDeviceFeatures m_enabled_features;

		VkQueue m_vk_graphics_queue;
        VkQueue m_vk_compute_queue;

        std::map<std::thread::id, ref<kb::vk::command_pool>> m_command_pools;

		bool m_destroyed = false;
	};
}

#endif
