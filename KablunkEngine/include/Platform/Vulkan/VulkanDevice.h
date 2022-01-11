#pragma once
#ifndef KABLUNK_PLATFORM_VULKAN_DEVICE_H
#define KABLUNK_PLATFORM_VULKAN_DEVICE_H

#include "Kablunk/Core/Core.h"
#include <vulkan/vulkan.h>

#include <optional>
#include <vector>

namespace Kablunk
{
	struct QueueFamilyIndices
	{
		std::optional<uint32_t> Graphics_family;
		std::optional<uint32_t> Present_family;

		bool HasGraphics() const { return Graphics_family.has_value(); }
		bool IsComplete() const { return Graphics_family.has_value() && Present_family.has_value(); }
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
	};

	// Logical Device
	class VulkanDevice : public RefCounted 
	{
	public:
		VulkanDevice(const IntrusiveRef<VulkanPhysicalDevice>& physical_device, VkPhysicalDeviceFeatures enabled_features);
		~VulkanDevice();

		void Destroy();

		VkQueue GetGraphicsQueue() { return m_graphics_queue; }
		
		VkCommandBuffer GetCommandBuffer(bool begin);
		void FlushCommandBuffer(VkCommandBuffer command_buffer);
		void FlushCommandBuffer(VkCommandBuffer command_buffer, VkQueue queue);

		VkCommandBuffer CreateSecondaryCommandBuffer();

		IntrusiveRef<VulkanPhysicalDevice> GetPhysicalDevice() { return m_physical_device; }
		VkPhysicalDevice GetVkPhysicalDevice() { return m_physical_device->GetVkDevice(); }
		VkDevice GetVkDevice() { return m_device; }
	private:
		VkDevice m_device;
		IntrusiveRef<VulkanPhysicalDevice> m_physical_device;
		VkPhysicalDeviceFeatures m_enabled_features;
		VkCommandPool m_command_pool;

		VkQueue m_graphics_queue;
	};
}

#endif
