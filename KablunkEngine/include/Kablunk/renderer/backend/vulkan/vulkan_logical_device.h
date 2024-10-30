#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_DEVICE_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_DEVICE_H

#include "Kablunk/Core/Core.h"

#include "kablunk/renderer/backend/vulkan/vulkan_command_pool.h"

#include <vulkan/vulkan.h>

#include <optional>
#include <vector>
#include <map>
#include <thread>

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

struct queue_family_indices_t
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
class vulkan_physical_device final : public RefCounted
{
public:
	vulkan_physical_device(VkInstance p_vk_instance);
    ~vulkan_physical_device() override = default;

	VkPhysicalDevice GetVkDevice() const { return m_device; }
	const queue_family_indices_t& GetQueueFamilyIndices() const { return m_queue_family_indices; }

	const VkPhysicalDeviceProperties& GetProperties() const { return m_properties; }
	const VkPhysicalDeviceLimits& GetLimits() const { return m_properties.limits; }
	const VkPhysicalDeviceFeatures& GetFeatures() const { return m_features; }
	const std::vector<const char*>& GetRequiredExtensions() const { return m_required_extensions; }
	VkFormat GetDepthFormat() const { return m_depth_format; }
private:
	void FindPresentingIndices(VkSurfaceKHR surface);
	queue_family_indices_t FindQueueFamilies(VkPhysicalDevice device);
	bool IsPhysicalDeviceSuitable(VkPhysicalDevice device);
	bool CheckDeviseExtensionSupport(VkPhysicalDevice device);
	std::vector<VkExtensionProperties> FindSupportedExtensions(VkPhysicalDevice device);
	VkFormat FindDepthFormat() const;
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

	queue_family_indices_t m_queue_family_indices;

	friend class vulkan_swap_chain;
    friend class vulkan_logical_device;
};

// Logical Device
class vulkan_logical_device final : public RefCounted
{
public:
	vulkan_logical_device(
        VkInstance p_vk_instance,
        const arc<vulkan_physical_device>& physical_device,
        VkPhysicalDeviceFeatures enabled_features
    );
	~vulkan_logical_device() override;

	void Destroy();

    auto get_vk_instance() const noexcept -> VkInstance { return m_vk_instance; }

	VkQueue get_vk_graphics_queue() const { return m_vk_graphics_queue; }
    VkQueue get_vk_compute_queue() const { return m_vk_compute_queue; }

	VkCommandBuffer get_vk_command_buffer(bool begin, bool p_compute = false);
	void flush_command_buffer(VkCommandBuffer command_buffer);
	void flush_command_buffer(VkCommandBuffer command_buffer, VkQueue queue, command_buffer_type_t p_command_buffer_type);

	VkCommandBuffer create_secondary_command_buffer();

	const arc<vulkan_physical_device>& get_physical_device() const noexcept { return m_physical_device; }
    auto get_physical_device() noexcept -> arc<vulkan_physical_device>& { return m_physical_device; }
	VkPhysicalDevice get_vk_physical_device() { return m_physical_device->GetVkDevice(); }
	VkDevice get_vk_device() const { return m_vk_device; }

private:
    arc<command_pool> get_thread_local_command_pool();
    arc<command_pool> get_or_create_thread_local_command_pool();

private:
    VkInstance m_vk_instance;
	VkDevice m_vk_device;
	arc<vulkan_physical_device> m_physical_device;
	VkPhysicalDeviceFeatures m_enabled_features;

	VkQueue m_vk_graphics_queue;
    VkQueue m_vk_compute_queue;

    std::map<std::thread::id, arc<command_pool>> m_command_pools;

	bool m_destroyed = false;
};

} // end namespace kb::render::backend::vk

#endif
