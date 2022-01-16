#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanTexture.h"

#include "Kablunk/Renderer/Renderer.h"

namespace Kablunk
{

	VulkanMaterial::VulkanMaterial(const IntrusiveRef<Shader>& shader, const std::string& name /*= ""*/)
		: m_shader{ shader }, m_name{ name }, m_write_descriptors(Renderer::GetConfig().frames_in_flight), m_dirty_descriptor_sets(Renderer::GetConfig().frames_in_flight)
	{
		Init();
		Renderer::RegisterShaderDependency(shader, this);
	}

	VulkanMaterial::VulkanMaterial(IntrusiveRef<Material> material, const std::string& name /*= ""*/)
		: m_shader{ material->GetShader() }, m_name{ name }, m_write_descriptors(Renderer::GetConfig().frames_in_flight), m_dirty_descriptor_sets(Renderer::GetConfig().frames_in_flight)
	{
		if (name.empty())
			m_name = material->GetName();

		Renderer::RegisterShaderDependency(m_shader, this);

		auto vulkan_material = material.As<VulkanMaterial>();
		m_uniform_storage_buffer = Buffer::Copy(vulkan_material->m_uniform_storage_buffer.get(), vulkan_material->m_uniform_storage_buffer.size());

		m_resident_descriptors = vulkan_material->m_resident_descriptors;
		m_resident_descriptor_array = vulkan_material->m_resident_descriptor_array;
		m_pending_descriptors = vulkan_material->m_pending_descriptors;
		m_textures = vulkan_material->m_textures;
		m_texture_array = vulkan_material->m_texture_array;
		m_images = vulkan_material->m_images;
		m_image_hashes = vulkan_material->m_image_hashes;
	}

	VulkanMaterial::~VulkanMaterial()
	{

	}

	void VulkanMaterial::Init()
	{
		AllocateStorage();

		m_material_flags |= static_cast<uint32_t>(MaterialFlag::DepthTest);
		m_material_flags |= static_cast<uint32_t>(MaterialFlag::Blend);


		IntrusiveRef<VulkanMaterial> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->Invalidate();
			});
	}

	void VulkanMaterial::Invalidate()
	{
		uint32_t frames_in_flight = Renderer::GetConfig().frames_in_flight;
		auto shader = m_shader.As<VulkanShader>();
		if (shader->HasDescriptorSet(0))
		{
			const auto& shader_descriptor_set = shader->GetShaderDescriptorSets();
			if (!shader_descriptor_set.empty())
				for (auto&& [binding, descriptor] : m_resident_descriptors)
					m_pending_descriptors.push_back(descriptor);
		}
	}

	void VulkanMaterial::Bind()
	{

	}

	void VulkanMaterial::Set(const std::string& name, float value)
	{
		Set<float>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, int value)
	{
		Set<int>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, uint32_t value)
	{
		Set<uint32_t>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, bool value)
	{
		Set<bool>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::vec2& value)
	{
		Set<glm::vec2>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::vec3& value)
	{
		Set<glm::vec3>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::vec4& value)
	{
		Set<glm::vec4>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::ivec2& value)
	{
		Set<glm::ivec2>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::ivec3& value)
	{
		Set<glm::ivec3>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::ivec4& value)
	{
		Set<glm::ivec4>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::mat3& value)
	{
		Set<glm::mat3>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const glm::mat4& value)
	{
		Set<glm::mat4>(name, value);
	}

	void VulkanMaterial::Set(const std::string& name, const IntrusiveRef<Texture2D>& texture)
	{
		SetVulkanDescriptor(name, texture);
	}

	void VulkanMaterial::Set(const std::string& name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index)
	{
		SetVulkanDescriptor(name, texture, array_index);
	}

	void VulkanMaterial::Set(const std::string& name, const IntrusiveRef<Image2D>& image)
	{
		SetVulkanDescriptor(name, image);
	}

	void VulkanMaterial::RT_UpdateForRendering(const std::vector<std::vector<VkWriteDescriptorSet>>& uniform_buffer_write_descriptors /*= std::vector<std::vector<VkWriteDescriptorSet>>()*/)
	{
		auto vulkan_device = VulkanContext::Get()->GetDevice();
		for (auto&& [binding, descriptor] : m_resident_descriptors)
		{
			if (descriptor->type == PendingDescriptorType::Image2D)
			{
				IntrusiveRef<VulkanImage2D> image = descriptor->image.As<VulkanImage2D>();
				KB_CORE_ASSERT(image->GetImageInfo().image_view, "image view is nullptr!");
				if (descriptor->write_descriptor_set.pImageInfo && image->GetImageInfo().image_view != descriptor->write_descriptor_set.pImageInfo->imageView)
				{
					m_pending_descriptors.emplace_back(descriptor);
					InvalidateDescriptorSets();
				}
			}
		}

		std::vector<VkDescriptorImageInfo> array_image_infos;

		uint32_t frame_index = Renderer::GetCurrentFrameIndex();

		// currently can't cache resources because the same material could be rendered in multiple viewports, so we can't bind to the same uniform buffer
		if (m_dirty_descriptor_sets[frame_index] || true)
		{
			m_dirty_descriptor_sets[frame_index] = false;
			m_write_descriptors[frame_index].clear();

			if (!uniform_buffer_write_descriptors.empty())
			{
				for (auto& wd : uniform_buffer_write_descriptors[frame_index])
					m_write_descriptors[frame_index].push_back(wd);
			}

			for (auto&& [binding, pending_descriptor] : m_resident_descriptors)
			{
				if (pending_descriptor->type == PendingDescriptorType::Texture2D)
				{
					IntrusiveRef<VulkanTexture2D> texture = pending_descriptor->texture.As<VulkanTexture2D>();
					pending_descriptor->image_info = texture->GetVulkanDescriptorInfo();
					pending_descriptor->write_descriptor_set.pImageInfo = &pending_descriptor->image_info;
				}
				else if (pending_descriptor->type == PendingDescriptorType::Image2D)
				{
					IntrusiveRef<VulkanImage2D> image = pending_descriptor->image.As<VulkanImage2D>();
					pending_descriptor->image_info = image->GetDescriptor();
					pending_descriptor->write_descriptor_set.pImageInfo = &pending_descriptor->image_info;
				}

				m_write_descriptors[frame_index].push_back(pending_descriptor->write_descriptor_set);
			}

			for (auto&& [binding, pending_descriptor] : m_resident_descriptor_array)
			{
				if (pending_descriptor->type == PendingDescriptorType::Texture2D)
				{
					for (size_t i = 0; i < pending_descriptor->textures.size(); ++i)
					{
						IntrusiveRef<Texture2D> texture = pending_descriptor->textures[i];
						KB_CORE_ASSERT(texture, "texture is uninitialized!");
						IntrusiveRef<VulkanTexture2D> vulkan_texture = texture.As<VulkanTexture2D>();
						array_image_infos.emplace_back(vulkan_texture->GetVulkanDescriptorInfo());
					}
				}

				pending_descriptor->write_descriptor_set.pImageInfo = array_image_infos.data();
				pending_descriptor->write_descriptor_set.descriptorCount = array_image_infos.size();
				m_write_descriptors[frame_index].push_back(pending_descriptor->write_descriptor_set);
			}
		}

		auto vulkan_shader = m_shader.As<VulkanShader>();
		auto descriptor_set = vulkan_shader->AllocateDescriptorSet();
		m_descriptor_sets[frame_index] = descriptor_set;
		for (auto& write_descriptor : m_write_descriptors[frame_index])
			write_descriptor.dstSet = descriptor_set.descriptor_sets[0];

		vkUpdateDescriptorSets(
			vulkan_device->GetVkDevice(), 
			static_cast<uint32_t>(m_write_descriptors[frame_index].size()), 
			m_write_descriptors[frame_index].data(), 
			0, 
			nullptr
		);

		m_pending_descriptors.clear();
	}

	void VulkanMaterial::InvalidateDescriptorSets()
	{
		const uint32_t framesInFlight = Renderer::GetConfig().frames_in_flight;
		for (uint32_t i = 0; i < framesInFlight; i++)
			m_dirty_descriptor_sets[i] = true;
	}

	void VulkanMaterial::AllocateStorage()
	{
		const auto& shader_buffers = m_shader->GetShaderBuffers();

		if (!shader_buffers.empty())
		{
			uint32_t size = 0;
			for (const auto& [name, shader_buffer] : shader_buffers)
				size += shader_buffer.size;

			m_uniform_storage_buffer.Allocate(size);
			m_uniform_storage_buffer.ZeroInitialize();
		}
	}

	void VulkanMaterial::OnShaderReloaded()
	{
		// #TODO implement
	}

	void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Texture2D>& texture)
	{
		const ShaderResourceDeclaration* resource = FindResourceDeclaration(name);
		KB_CORE_ASSERT(resource, "resource is nullptr!");

		uint32_t binding = resource->GetRegister();
		// Texture already bound!
		if (binding < m_textures.size() && m_textures[binding] && texture->GetHash() == m_textures[binding]->GetHash())
			return;

		if (binding >= m_textures.size())
			m_textures.resize(binding + 1);

		m_textures[binding] = texture->GetImage();

		const VkWriteDescriptorSet* write_descriptor_set = m_shader.As<VulkanShader>()->GetDescriptorSet(name);
		KB_CORE_ASSERT(write_descriptor_set, "nullptr!");

		m_resident_descriptors[binding] = std::make_shared<PendingDescriptor>(
			PendingDescriptor
			{
				PendingDescriptorType::Texture2D,
				*write_descriptor_set,
				{},
				texture.As<Texture>()
			}
		);

		m_pending_descriptors.push_back(m_resident_descriptors.at(binding));

		InvalidateDescriptorSets();
	}

	void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Texture2D>& texture, uint32_t array_index)
	{
		const ShaderResourceDeclaration* resource = FindResourceDeclaration(name);
		KB_CORE_ASSERT(resource, "resource is nullptr!");

		uint32_t binding = resource->GetRegister();
		// Texture already bound!
		if (binding < m_textures.size() && m_textures[binding] && texture->GetHash() == m_texture_array[binding][array_index]->GetHash())
			return;

		if (binding >= m_textures.size())
			m_texture_array.resize(binding + 1);

		if (array_index >= m_texture_array[binding].size())
			m_texture_array[binding].resize(array_index + 1);

		m_texture_array[binding][array_index] = texture;

		const VkWriteDescriptorSet* write_descriptor_set = m_shader.As<VulkanShader>()->GetDescriptorSet(name);
		KB_CORE_ASSERT(write_descriptor_set, "nullptr!");

		if (m_resident_descriptor_array.find(binding) == m_resident_descriptor_array.end())
		{
			m_resident_descriptor_array[binding] = std::make_shared<PendingDescriptorArray>(
				PendingDescriptorArray
				{
					PendingDescriptorType::Texture2D,
					*write_descriptor_set,
					{},
					{},
					{}
				}
			);
		}

		auto& resident_descriptor_array = m_resident_descriptor_array.at(binding);
		if (array_index >= resident_descriptor_array->textures.size())
			resident_descriptor_array->textures.resize(array_index + 1);

		resident_descriptor_array->textures[array_index] = texture;

		InvalidateDescriptorSets();
	}

	void VulkanMaterial::SetVulkanDescriptor(const std::string& name, const IntrusiveRef<Image2D>& image)
	{
		KB_CORE_ASSERT(image, "image is nullptr!");
		KB_CORE_ASSERT(image.As<VulkanImage2D>()->GetImageInfo().image_view, "ImageView is nullptr!");

		const ShaderResourceDeclaration* resource = FindResourceDeclaration(name);
		KB_CORE_ASSERT(resource, "resource is null!");

		uint32_t binding = resource->GetRegister();
		// TODO: replace with set/map
		if (binding < m_images.size() && m_images[binding] && m_image_hashes.at(binding) == image->GetHash())
			return;

		if (resource->GetRegister() >= m_images.size())
			m_images.resize(resource->GetRegister() + 1);

		m_images[resource->GetRegister()] = image;
		m_image_hashes[resource->GetRegister()] = image->GetHash();

		const VkWriteDescriptorSet* write_descriptor_set = m_shader.As<VulkanShader>()->GetDescriptorSet(name);
		KB_CORE_ASSERT(write_descriptor_set, "nullptr!");
		m_resident_descriptors[binding] = std::make_shared<PendingDescriptor>(
			PendingDescriptor
			{ 
				PendingDescriptorType::Image2D, 
				*write_descriptor_set, 
				{}, 
				nullptr, 
				image.As<Image>() 
			}
		);

		m_pending_descriptors.push_back(m_resident_descriptors.at(binding));

		InvalidateDescriptorSets();
	}

	const ShaderUniform* VulkanMaterial::FindUniformDeclaration(const std::string& name)
	{
		const auto& shader_buffer = m_shader->GetShaderBuffers();

		KB_CORE_ASSERT(shader_buffer.size() <= 1, "only one shader buffer is currently supported!");

		if (!shader_buffer.empty())
		{
			const ShaderBuffer& buffer = shader_buffer.begin()->second;
			if (buffer.uniforms.find(name) != buffer.uniforms.end())
				return &buffer.uniforms.at(name);
			else
				return nullptr;
		}
		else
			return nullptr;
	}

	const ShaderResourceDeclaration* VulkanMaterial::FindResourceDeclaration(const std::string& name)
	{
		const auto& resources = m_shader->GetResources();
		for (const auto& [nname, resource] : resources) // #TODO why can't we just access via indexing? 
			if (resource.GetName() == name)
				return &resource;
		
		return nullptr;
	}

	float& VulkanMaterial::GetFloat(const std::string& name)
	{
		return Get<float>(name);
	}

	int& VulkanMaterial::GetInt(const std::string& name)
	{
		return Get<int>(name);
	}

	uint32_t& VulkanMaterial::GetUInt(const std::string& name)
	{
		return Get<uint32_t>(name);
	}

	glm::vec2& VulkanMaterial::GetVec2(const std::string& name)
	{
		return Get<glm::vec2>(name);
	}

	glm::vec3& VulkanMaterial::GetVec3(const std::string& name)
	{
		return Get<glm::vec3>(name);
	}

	glm::vec4& VulkanMaterial::GetVec4(const std::string& name)
	{
		return Get<glm::vec4>(name);
	}

	glm::mat3& VulkanMaterial::GetMat3(const std::string& name)
	{
		return Get<glm::mat3>(name);
	}

	glm::mat4& VulkanMaterial::GetMat4(const std::string& name)
	{
		return Get<glm::mat4>(name);
	}

	IntrusiveRef<Texture2D> VulkanMaterial::GetTexture2D(const std::string& name)
	{
		return GetResource<Texture2D>(name);
	}

	IntrusiveRef<Texture2D> VulkanMaterial::TryGetTexture2D(const std::string& name)
	{
		return TryGetResource<Texture2D>(name);
	}

}
