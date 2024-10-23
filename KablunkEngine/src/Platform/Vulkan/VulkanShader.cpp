#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanShader.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Kablunk/Renderer/RenderCommand.h"


#include "Kablunk/Renderer/ShaderCache.h"

#include <shaderc/shaderc.hpp>
#include <spirv_cross/spirv_glsl.hpp>
#include <spirv-tools/libspirv.h>


#include <filesystem>

namespace kb
{ // start namespace kb

namespace Internal
{ // start namespace ::Internal
	static const char* GetCacheDirectory() { return "Resources/Cache/Shader/Vulkan"; }

	static void CreateCacheDirectoryIfNecessary()
	{
        const std::string cache_dir = GetCacheDirectory();
		if (!std::filesystem::exists(cache_dir))
			std::filesystem::create_directories(cache_dir);
	}

	static ShaderUniformType SPIRTypeToShaderUniformType(spirv_cross::SPIRType type)
	{
		switch (type.basetype)
		{
		case spirv_cross::SPIRType::Boolean:  return ShaderUniformType::Bool;
		case spirv_cross::SPIRType::Int:
			if (type.vecsize == 1)            return ShaderUniformType::Int;
			if (type.vecsize == 2)            return ShaderUniformType::IVec2;
			if (type.vecsize == 3)            return ShaderUniformType::IVec3;
			if (type.vecsize == 4)            return ShaderUniformType::IVec4;

		case spirv_cross::SPIRType::UInt:     return ShaderUniformType::UInt;
		case spirv_cross::SPIRType::Float:
			if (type.columns == 3)            return ShaderUniformType::Mat3;
			if (type.columns == 4)            return ShaderUniformType::Mat4;

			if (type.vecsize == 1)            return ShaderUniformType::Float;
			if (type.vecsize == 2)            return ShaderUniformType::Vec2;
			if (type.vecsize == 3)            return ShaderUniformType::Vec3;
			if (type.vecsize == 4)            return ShaderUniformType::Vec4;
			break;
		}
		KB_CORE_ASSERT(false, "Unknown type!");
		return ShaderUniformType::None;
	}

	static std::string ReadShaderFromFile(const std::string& filepath)
	{
		std::string result;
		std::ifstream in(filepath, std::ios::in | std::ios::binary);
		if (in)
		{
			in.seekg(0, std::ios::end);
			result.resize(in.tellg());
			in.seekg(0, std::ios::beg);
			in.read(&result[0], result.size());
		}
		else
			KB_CORE_ASSERT(false, "Could not load shader!");

		in.close();

		return result;
	}

	static const char* VkShaderStageCachedFileExtension(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:    return ".cached_vulkan.vert";
		case VK_SHADER_STAGE_FRAGMENT_BIT:  return ".cached_vulkan.frag";
		case VK_SHADER_STAGE_COMPUTE_BIT:   return ".cached_vulkan.comp";
		}
		KB_CORE_ASSERT(false, "unreachable");
		return "";
	}

	static shaderc_shader_kind VkShaderStageToShaderC(VkShaderStageFlagBits stage)
	{
		switch (stage)
		{
		case VK_SHADER_STAGE_VERTEX_BIT:    return shaderc_vertex_shader;
		case VK_SHADER_STAGE_FRAGMENT_BIT:  return shaderc_fragment_shader;
		case VK_SHADER_STAGE_COMPUTE_BIT:   return shaderc_compute_shader;
		}
		KB_CORE_ASSERT(false, "unreachable");
		return (shaderc_shader_kind)0;
	}

	static VkShaderStageFlagBits ShaderTypeFromString(const std::string& type)
	{
		if (type == "vertex")                       return VK_SHADER_STAGE_VERTEX_BIT;
		if (type == "fragment" || type == "pixel")  return VK_SHADER_STAGE_FRAGMENT_BIT;
		if (type == "compute")                      return VK_SHADER_STAGE_COMPUTE_BIT;

		return VK_SHADER_STAGE_FLAG_BITS_MAX_ENUM;
	}
} // end namespace ::Internal

static kb::unordered_flat_map<uint32_t, kb::unordered_flat_map<uint32_t, VulkanShader::UniformBuffer*>> s_uniform_buffers;
static kb::unordered_flat_map<uint32_t, kb::unordered_flat_map<uint32_t, VulkanShader::StorageBuffer*>> s_storage_buffers;


VulkanShader::VulkanShader(const std::string& path, bool force_compile)
	: m_file_path{ path }, m_hash{ 0 }
{
    KB_PROFILE_SCOPE;

    m_hash = std::hash<std::string>{}(m_file_path);

	// Get name from filepath
	size_t found = path.find_last_of("/\\");
	m_name = found != std::string::npos ? path.substr(found + 1) : path;
	found = m_name.find_last_of(".");
	m_name = found != std::string::npos ? m_name.substr(0, found) : m_name;

	KB_CORE_INFO("Creating Vulkan Shader with name: {0}", m_name);

	Reload(force_compile);
}

void VulkanShader::destroy()
{
    KB_PROFILE_SCOPE;

	if (m_has_destroyed)
		return;

    arc<VulkanShader> instance{ this };
	render::submit_resource_free([instance]()
		{
            const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
			for (const auto& pipeline_create_info : instance->m_pipeline_shader_stage_create_infos)
				vkDestroyShaderModule(device, pipeline_create_info.module, nullptr);
		}
	);

	m_has_destroyed = true;
}

void VulkanShader::Reload(bool force_compile /*= false*/)
{
    KB_PROFILE_SCOPE;

    arc instance{ this };
	render::submit([inst = instance, force_compile]() mutable
	    {
		    // Clear old shader data
            inst->m_shader_descriptor_sets.clear();
            inst->m_resources.clear();
            inst->m_push_constant_ranges.clear();
            inst->m_pipeline_shader_stage_create_infos.clear();
            inst->m_descriptor_set_layouts.clear();
            inst->m_shader_source.clear();
            inst->m_buffers.clear();
            inst->m_type_counts.clear();

		    Internal::CreateCacheDirectoryIfNecessary();

            const std::string source = Internal::ReadShaderFromFile(inst->m_file_path);
		    force_compile = ShaderCache::HasChanged(inst->m_file_path, source);

            inst->m_shader_source = inst->PreProcess(source);
		    kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>> shader_data;
            inst->CompileOrGetVulkanBinaries(shader_data, force_compile);
            inst->LoadAndCreateShaders(shader_data);
            inst->ReflectAllShaderStages(shader_data);
            inst->CreateDescriptors();

		    render::on_shader_reloaded(inst->GetHash());
	    });
}

size_t VulkanShader::GetHash() const
{
	return m_hash;
}

void VulkanShader::AddShaderReloadedCallback(const ShaderReloadedCallback& callback)
{

}

void VulkanShader::Bind() const
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::Unbind() const
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetMat4(const std::string& name, const glm::mat4& value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetFloat(const std::string& name, float value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetFloat2(const std::string& name, const glm::vec2& value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetFloat3(const std::string& name, const glm::vec3& value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetFloat4(const std::string& name, const glm::vec4& value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetInt(const std::string& name, int value)
{
	KB_CORE_ASSERT(false, "not implemented!");
}

void VulkanShader::SetIntArray(const std::string& name, int* values, uint32_t count)
{
	KB_CORE_ERROR("VulkanShader SetIntArray() not implemented!");
}

std::vector<VkDescriptorSetLayout> VulkanShader::GetAllDescriptorSetLayouts()
{
    KB_PROFILE_SCOPE;

	std::vector<VkDescriptorSetLayout> result;
	result.reserve(m_descriptor_set_layouts.size());

	for (auto& layout : m_descriptor_set_layouts)
		result.emplace_back(layout);

	return result;
}

kb::VulkanShader::ShaderMaterialDescriptorSet VulkanShader::allocate_descriptor_set(uint32_t set /*= 0*/) const
{
    KB_PROFILE_SCOPE;

	KB_CORE_ASSERT(set < m_descriptor_set_layouts.size(), "set outside of map bounds");
	ShaderMaterialDescriptorSet result;

	if (m_shader_descriptor_sets.empty())
		return result;

	result.pool = nullptr;

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &m_descriptor_set_layouts.at(set);

    const auto vulkan_render_backend = render::Renderer::get().get_render_backend().backend();

    const VkDescriptorSet vk_descriptor_set = vulkan_render_backend->rt_allocate_descriptor_set(alloc_info);
	KB_CORE_ASSERT(vk_descriptor_set, "Vulkan failed to allocate descriptor set!");
	result.descriptor_sets.push_back(vk_descriptor_set);

	return result;
}

VulkanShader::ShaderMaterialDescriptorSet VulkanShader::CreateDescriptorSets(uint32_t set /*= 0*/)
{
    KB_PROFILE_SCOPE;

    kb::log::core::trace(
        log::logger_tag_t::shader,
        "Creating descriptor set {} for '{}'",
        set,
        m_name
    );

	ShaderMaterialDescriptorSet result;

    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

	// #TODO Move this to the centralized renderer
	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.pNext = nullptr;
	descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(m_type_counts.at(set).size());
	descriptor_pool_create_info.pPoolSizes = m_type_counts.at(set).data();
	descriptor_pool_create_info.maxSets = 1;

	if (vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &result.pool) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "failed to create descriptor pool info!");

	VkDescriptorSetAllocateInfo alloc_info = {};
	alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	alloc_info.descriptorPool = result.pool;
	alloc_info.descriptorSetCount = 1;
	alloc_info.pSetLayouts = &m_descriptor_set_layouts.at(set);

	result.descriptor_sets.emplace_back();
	if (vkAllocateDescriptorSets(device, &alloc_info, result.descriptor_sets.data()) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to allocate descriptor sets!");

	return result;
}

VulkanShader::ShaderMaterialDescriptorSet VulkanShader::CreateDescriptorSets(uint32_t set, uint32_t number_of_sets)
{
    KB_PROFILE_SCOPE;

	ShaderMaterialDescriptorSet result;

    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

	unordered_flat_map<uint32_t, std::vector<VkDescriptorPoolSize>> pool_sizes;
	for (uint32_t descriptor_set = 0; descriptor_set < m_shader_descriptor_sets.size(); descriptor_set++)
	{
		auto& shader_descriptor_set = m_shader_descriptor_sets[descriptor_set];
		if (!shader_descriptor_set) // Empty descriptor set
			continue;

		if (!shader_descriptor_set.uniform_buffers.empty())
		{
			VkDescriptorPoolSize& type_count = pool_sizes[descriptor_set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.uniform_buffers.size()) * number_of_sets;
		}
		if (!shader_descriptor_set.storage_buffers.empty())
		{
			VkDescriptorPoolSize& type_count = pool_sizes[descriptor_set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.storage_buffers.size()) * number_of_sets;
		}
		if (!shader_descriptor_set.image_samplers.empty())
		{
			VkDescriptorPoolSize& type_count = pool_sizes[descriptor_set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			uint32_t descriptor_set_count = 0;
			for (auto&& [binding, image_sampler] : shader_descriptor_set.image_samplers)
				descriptor_set_count += image_sampler.array_size;

			type_count.descriptorCount = descriptor_set_count * number_of_sets;
		}
		if (!shader_descriptor_set.storage_images.empty())
		{
			VkDescriptorPoolSize& type_count = pool_sizes[descriptor_set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.storage_images.size()) * number_of_sets;
		}
	}

	KB_CORE_ASSERT(pool_sizes.find(set) != pool_sizes.end(), "set not found in pool!");

	// #TODO Move this to the centralized renderer
	VkDescriptorPoolCreateInfo descriptor_pool_create_info = {};
	descriptor_pool_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	descriptor_pool_create_info.pNext = nullptr;
	descriptor_pool_create_info.poolSizeCount = static_cast<uint32_t>(pool_sizes.at(set).size());
	descriptor_pool_create_info.pPoolSizes = pool_sizes.at(set).data();
	descriptor_pool_create_info.maxSets = number_of_sets;

	if (vkCreateDescriptorPool(device, &descriptor_pool_create_info, nullptr, &result.pool) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create descriptor pool!");

	result.descriptor_sets.resize(number_of_sets);

	for (uint32_t i = 0; i < number_of_sets; i++)
	{
		VkDescriptorSetAllocateInfo alloc_info = {};
		alloc_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
		alloc_info.descriptorPool = result.pool;
		alloc_info.descriptorSetCount = 1;
		alloc_info.pSetLayouts = &m_descriptor_set_layouts.at(set);

		if (vkAllocateDescriptorSets(device, &alloc_info, &result.descriptor_sets[i]) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to allocate descriptor sets!");
	}
	return result;

}

const VkWriteDescriptorSet* VulkanShader::GetDescriptorSet(const std::string& name, uint32_t set /*= 0*/) const
{
	KB_CORE_ASSERT(set < m_shader_descriptor_sets.size(), "error");
	KB_CORE_ASSERT(m_shader_descriptor_sets[set], "error");
	if (m_shader_descriptor_sets.at(set).write_descriptor_sets.find(name) == m_shader_descriptor_sets.at(set).write_descriptor_sets.end())
	{
		KB_CORE_WARN("Shader {0} does not contain requested descriptor set {1}", m_name, name);
		return nullptr;
	}
	return &m_shader_descriptor_sets.at(set).write_descriptor_sets.at(name);

}

void VulkanShader::ClearUniformBuffers()
{
	s_uniform_buffers.clear();
	s_storage_buffers.clear();
}

unordered_flat_map<VkShaderStageFlagBits, std::string> VulkanShader::PreProcess(const std::string& source)
{
	unordered_flat_map<VkShaderStageFlagBits, std::string> shader_sources;

    const auto type_token = "#type";
    const size_t type_token_len = strlen(type_token);
	size_t pos = source.find(type_token, 0);
	while (pos != std::string::npos)
	{
        const size_t eol = source.find_first_of("\r\n", pos);
		KB_CORE_ASSERT(eol != std::string::npos, "Syntax error");
        const size_t begin = pos + type_token_len + 1;
		std::string type = source.substr(begin, eol - begin);
		KB_CORE_ASSERT(type == "vertex" || type == "fragment" || type == "pixel" || type == "compute", "Invalid shader type specified");

        const size_t next_line_pos = source.find_first_not_of("\r\n", eol);
		pos = source.find(type_token, next_line_pos);
		auto shader_type = Internal::ShaderTypeFromString(type);
		shader_sources[shader_type] = source.substr(next_line_pos, pos - (next_line_pos == std::string::npos ? source.size() - 1 : next_line_pos));
	}

	return shader_sources;
}

void VulkanShader::CompileOrGetVulkanBinaries(unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& output_binary, bool force_compile)
{
    KB_PROFILE_SCOPE;

	std::filesystem::path cache_dir = Internal::GetCacheDirectory();
	for (const auto& [stage, source] : m_shader_source)
	{
		auto extension = Internal::VkShaderStageCachedFileExtension(stage);
		if (!force_compile)
		{
			std::filesystem::path p = m_file_path;
			auto path = cache_dir / (p.filename().string() + extension);
			std::string cached_file_path = path.string();

			// #TODO implement with filesystem instead of c
			FILE* f;
			errno_t err = fopen_s(&f, cached_file_path.c_str(), "rb");
			if (!err)
			{
				fseek(f, 0, SEEK_END);
				uint64_t size = ftell(f);
				fseek(f, 0, SEEK_SET);
				output_binary[stage] = std::vector<uint32_t>(size / sizeof(uint32_t));
				fread(output_binary[stage].data(), sizeof(uint32_t), output_binary[stage].size(), f);
				fclose(f);
			}
		}

		if (output_binary[stage].empty())
		{
			shaderc::Compiler compiler;
			shaderc::CompileOptions options;
			options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
			options.SetWarningsAsErrors();
			options.SetGenerateDebugInfo();

#ifdef KB_DEBUG
			constexpr bool optimize = false;
#else
			constexpr bool optimize = true;
#endif
			if (optimize)
				options.SetOptimizationLevel(shaderc_optimization_level_performance);

			auto& shader_source = m_shader_source.at(stage);
			shaderc::SpvCompilationResult module = compiler.CompileGlslToSpv(shader_source, Internal::VkShaderStageToShaderC(stage), m_file_path.c_str());

			if (module.GetCompilationStatus() != shaderc_compilation_status_success)
			{
				KB_CORE_ERROR("{0}", module.GetErrorMessage());
				KB_CORE_ASSERT(false, "module compilation failed");
			}

            auto begin = reinterpret_cast<const uint8_t*>(module.cbegin());
            auto end = reinterpret_cast<const uint8_t*>(module.cend());
			const ptrdiff_t size = end - begin;

			output_binary[stage] = std::vector<uint32_t>(module.cbegin(), module.cend());

			std::filesystem::path p = m_file_path;
			auto path = cache_dir / (p.filename().string() + extension);
			std::string cached_filepath = path.string();

			// #TODO replace with filesystem instead of c
			FILE* f;
			errno_t err = fopen_s(&f, cached_filepath.c_str(), "wb");
			if (!err)
			{
				fwrite(output_binary[stage].data(), sizeof(uint32_t), output_binary[stage].size(), f);
				fclose(f);
			}
		}
	}
}

void VulkanShader::LoadAndCreateShaders(const kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data)
{
    KB_PROFILE_SCOPE;

    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

	m_pipeline_shader_stage_create_infos.clear();
	for (auto [stage, data] : shader_data)
	{
		KB_CORE_ASSERT(!data.empty(), "No Data in shader map!");

		VkShaderModuleCreateInfo module_create_info{};
		module_create_info.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		module_create_info.codeSize = data.size() * sizeof(uint32_t);
		module_create_info.pCode = data.data();

		VkShaderModule shader_module;
		if (vkCreateShaderModule(device, &module_create_info, nullptr, &shader_module) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create shader module!");

		VkPipelineShaderStageCreateInfo& shader_stage = m_pipeline_shader_stage_create_infos.emplace_back();
		shader_stage.sType  = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shader_stage.stage  = stage;
		shader_stage.module = shader_module;
		shader_stage.pName  = "main";
	}
}

void VulkanShader::Reflect(VkShaderStageFlagBits shader_stage, const std::vector<uint32_t>& shader_data)
{
	KB_CORE_TRACE("===========================");
	KB_CORE_TRACE(" Vulkan Shader Reflection");
	KB_CORE_TRACE(" {0}", m_file_path);
	KB_CORE_TRACE("===========================");

    const spirv_cross::Compiler compiler{ shader_data };
	auto resources = compiler.get_shader_resources();

	KB_CORE_TRACE("Uniform Buffers:");
	for (const auto& resource : resources.uniform_buffers)
	{
		const auto& name = resource.name;
		auto& buffer_type = compiler.get_type(resource.base_type_id);
        u32 member_count = static_cast<u32>(buffer_type.member_types.size());
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));

		if (descriptor_set >= m_shader_descriptor_sets.size())
			m_shader_descriptor_sets.resize(descriptor_set + 1);

		ShaderDescriptorSet& shader_descriptor_set = m_shader_descriptor_sets[descriptor_set];
		if (s_uniform_buffers[descriptor_set].find(binding) == s_uniform_buffers[descriptor_set].end())
		{
            const auto uniform_buffer = new UniformBuffer();
			uniform_buffer->binding_point = binding;
			uniform_buffer->size = size;
			uniform_buffer->name = name;
			uniform_buffer->shader_stage = VK_SHADER_STAGE_ALL;
			s_uniform_buffers.at(descriptor_set)[binding] = uniform_buffer;
		}
		else
		{
			UniformBuffer* uniform_buffer = s_uniform_buffers.at(descriptor_set).at(binding);
			if (size > uniform_buffer->size)
				uniform_buffer->size = size;
		}

		shader_descriptor_set.uniform_buffers[binding] = s_uniform_buffers.at(descriptor_set).at(binding);

		KB_CORE_TRACE("  {0} ({1}, {2})", name, descriptor_set, binding);
		KB_CORE_TRACE("  Member Count: {0}", member_count);
		KB_CORE_TRACE("  Size: {0}", size);
		KB_CORE_TRACE("-------------------");
	}

	KB_CORE_TRACE("Storage Buffers:");
	for (const auto& resource : resources.storage_buffers)
	{
		const auto& name = resource.name;
		auto& buf_type = compiler.get_type(resource.base_type_id);
        u32 member_count = static_cast<u32>(buf_type.member_types.size());
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        u32 size = static_cast<u32>(compiler.get_declared_struct_size(buf_type));

		if (descriptor_set >= m_shader_descriptor_sets.size())
			m_shader_descriptor_sets.resize(descriptor_set + 1);

		ShaderDescriptorSet& shader_descriptor_set = m_shader_descriptor_sets[descriptor_set];
		if (s_storage_buffers[descriptor_set].find(binding) == s_storage_buffers[descriptor_set].end())
		{
            const auto storage_buffer = new StorageBuffer{};
			storage_buffer->binding_point = binding;
			storage_buffer->size = size;
			storage_buffer->name = name;
			storage_buffer->shader_stage = VK_SHADER_STAGE_ALL;
			s_storage_buffers.at(descriptor_set)[binding] = storage_buffer;
		}
		else
		{
            const auto storage_buffer = s_storage_buffers.at(descriptor_set).at(binding);
			if (size > storage_buffer->size)
				storage_buffer->size = size;
		}

		shader_descriptor_set.storage_buffers[binding] = s_storage_buffers.at(descriptor_set).at(binding);

		KB_CORE_TRACE("  {0} ({1}, {2})", name, descriptor_set, binding);
		KB_CORE_TRACE("  Member Count: {0}", member_count);
		KB_CORE_TRACE("  Size: {0}", size);
		KB_CORE_TRACE("-------------------");
	}

	KB_CORE_TRACE("Push Constant Buffers:");
	for (const auto& resource : resources.push_constant_buffers)
	{
		const auto& buffer_name = resource.name;
		auto& buffer_type = compiler.get_type(resource.base_type_id);
		auto buffer_size = static_cast<u32>(compiler.get_declared_struct_size(buffer_type));
        const u32 member_count = static_cast<u32>(buffer_type.member_types.size());
        u32 buffer_offset = 0;
		if (!m_push_constant_ranges.empty())
			buffer_offset = m_push_constant_ranges.back().offset + m_push_constant_ranges.back().size;

		auto& push_constant_range = m_push_constant_ranges.emplace_back();
		push_constant_range.shader_stage = shader_stage;
		push_constant_range.size = buffer_size;
		push_constant_range.offset = buffer_offset;
        /*KB_CORE_ASSERT(
            buffer_size > buffer_offset,
            "[VulkanShader]: integer overflow when computing push constant size!"
        )*/

		// Skip empty push constant buffers - these are for the renderer only
		if (buffer_name.empty() || buffer_name == "u_Renderer")
			continue;

		ShaderBuffer& buffer = m_buffers[buffer_name];
		buffer.name = buffer_name;
		buffer.size = buffer_size - buffer_offset;

		KB_CORE_TRACE("  Name: {0}", buffer_name);
		KB_CORE_TRACE("  Member Count: {0}", member_count);
		KB_CORE_TRACE("  Size: {0}", buffer_size);
        KB_CORE_TRACE("  Offset: {}", buffer_offset);

		for (u32 i = 0; i < member_count; i++)
		{
			const auto& type = compiler.get_type(buffer_type.member_types[i]);
			const auto& member_name = compiler.get_member_name(buffer_type.self, i);
			const auto size = static_cast<u32>(compiler.get_declared_struct_member_size(buffer_type, i));
			const auto offset = compiler.type_struct_member_offset(buffer_type, i) - buffer_offset;

			std::string uniform_name = fmt::format("{}.{}", buffer_name, member_name);
			buffer.uniforms[uniform_name] = ShaderUniform(uniform_name, Internal::SPIRTypeToShaderUniformType(type), size, offset);
		}
	}

	KB_CORE_TRACE("Sampled Images:");
	for (const auto& resource : resources.sampled_images)
	{
		const auto& name = resource.name;
		auto& base_type = compiler.get_type(resource.base_type_id);
		auto& type = compiler.get_type(resource.type_id);
		u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
		const u32 dimension = base_type.image.dim;
		u32 array_size = type.array[0];
		if (array_size == 0)
			array_size = 1;
		if (descriptor_set >= m_shader_descriptor_sets.size())
			m_shader_descriptor_sets.resize(descriptor_set + 1);

		ShaderDescriptorSet& shader_descriptor_set = m_shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.image_samplers[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.shader_stage = shader_stage;
        // #TODO do we need both array size and dimension?
		image_sampler.array_size = array_size;
        image_sampler.m_dimension = dimension;

        m_resources[name] = ShaderResourceDeclaration{ name, descriptor_set,binding, 1 };

		KB_CORE_TRACE("  {0} ({1}, {2})", name, descriptor_set, binding);
	}

	KB_CORE_TRACE("Storage Images:");
	for (const auto& resource : resources.storage_images)
	{
		const auto& name = resource.name;
		auto& type = compiler.get_type(resource.base_type_id);
        u32 binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
        u32 descriptor_set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
        const u32 dimension = type.image.dim;

		if (descriptor_set >= m_shader_descriptor_sets.size())
			m_shader_descriptor_sets.resize(descriptor_set + 1);

		ShaderDescriptorSet& shader_descriptor_set = m_shader_descriptor_sets[descriptor_set];
		auto& image_sampler = shader_descriptor_set.storage_images[binding];
		image_sampler.binding_point = binding;
		image_sampler.descriptor_set = descriptor_set;
		image_sampler.name = name;
		image_sampler.shader_stage = shader_stage;
        image_sampler.m_dimension = dimension;

        m_resources[name] = ShaderResourceDeclaration{ name, descriptor_set, binding, 1 };

		KB_CORE_TRACE("  {0} ({1}, {2})", name, descriptor_set, binding);
	}

	KB_CORE_TRACE("===========================");
}

void VulkanShader::ReflectAllShaderStages(const kb::unordered_flat_map<VkShaderStageFlagBits, std::vector<uint32_t>>& shader_data)
{
	for (auto [stage, data] : shader_data)
		Reflect(stage, data);
}

void VulkanShader::CreateDescriptors()
{
    KB_PROFILE_SCOPE;

    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();

	m_type_counts.clear();
	for (uint32_t set = 0; set < m_shader_descriptor_sets.size(); ++set)
	{
		auto& shader_descriptor_set = m_shader_descriptor_sets[set];

		if (!shader_descriptor_set.uniform_buffers.empty())
		{
			VkDescriptorPoolSize& type_count = m_type_counts[set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.uniform_buffers.size());
		}

		if (!shader_descriptor_set.storage_buffers.empty())
		{
			VkDescriptorPoolSize& type_count = m_type_counts[set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.storage_buffers.size());
		}

		if (!shader_descriptor_set.image_samplers.empty())
		{
			VkDescriptorPoolSize& type_count = m_type_counts[set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.image_samplers.size());
		}


		if (!shader_descriptor_set.storage_images.empty())
		{
			VkDescriptorPoolSize& type_count = m_type_counts[set].emplace_back();
			type_count.type = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			type_count.descriptorCount = static_cast<uint32_t>(shader_descriptor_set.storage_images.size());
		}

		// Uniform Buffers
		std::vector<VkDescriptorSetLayoutBinding> layout_bindings;
		for (auto& [binding, uniform_buffer] : shader_descriptor_set.uniform_buffers)
		{
			VkDescriptorSetLayoutBinding& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
			layout_binding.descriptorCount = 1;
			layout_binding.stageFlags = uniform_buffer->shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			VkWriteDescriptorSet& vk_descriptor_set = shader_descriptor_set.write_descriptor_sets[uniform_buffer->name];
            vk_descriptor_set = {};
            vk_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_descriptor_set.descriptorType = layout_binding.descriptorType;
            vk_descriptor_set.descriptorCount = 1;
            vk_descriptor_set.dstBinding = layout_binding.binding;
		}

		// Storage Buffers
		for (auto& [binding, storage_buffer] : shader_descriptor_set.storage_buffers)
		{
			VkDescriptorSetLayoutBinding& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
			layout_binding.descriptorCount = 1;
			layout_binding.stageFlags = storage_buffer->shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			VkWriteDescriptorSet& vk_descriptor_set = shader_descriptor_set.write_descriptor_sets[storage_buffer->name];
            vk_descriptor_set = {};
            vk_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_descriptor_set.descriptorType = layout_binding.descriptorType;
            vk_descriptor_set.descriptorCount = 1;
            vk_descriptor_set.dstBinding = layout_binding.binding;
		}

		// Image Samplers
		for (auto& [binding, image_sampler] : shader_descriptor_set.image_samplers)
		{
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			layout_binding.descriptorCount = image_sampler.array_size;
			layout_binding.stageFlags = image_sampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;
			layout_binding.binding = binding;

			KB_CORE_ASSERT(shader_descriptor_set.uniform_buffers.find(binding) == shader_descriptor_set.uniform_buffers.end(), "Binding is already present!");
			KB_CORE_ASSERT(shader_descriptor_set.storage_buffers.find(binding) == shader_descriptor_set.storage_buffers.end(), "Binding is already present!");

			VkWriteDescriptorSet& vk_descriptor_set = shader_descriptor_set.write_descriptor_sets[image_sampler.name];
            vk_descriptor_set = {};
            vk_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_descriptor_set.descriptorType = layout_binding.descriptorType;
            vk_descriptor_set.descriptorCount = layout_binding.descriptorCount;
            vk_descriptor_set.dstBinding = layout_binding.binding;
		}

		// Storage Images
		for (auto& [binding_and_set, image_sampler] : shader_descriptor_set.storage_images)
		{
			auto& layout_binding = layout_bindings.emplace_back();
			layout_binding.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_IMAGE;
			layout_binding.descriptorCount = image_sampler.array_size;
			layout_binding.stageFlags = image_sampler.shader_stage;
			layout_binding.pImmutableSamplers = nullptr;

			// mask
			uint32_t binding = binding_and_set & 0xffffffff;
			layout_binding.binding = binding;

			KB_CORE_ASSERT(
                shader_descriptor_set.uniform_buffers.find(binding) == shader_descriptor_set.uniform_buffers.end(),
                "Binding is already present!"
            );
			KB_CORE_ASSERT(
                shader_descriptor_set.storage_buffers.find(binding) == shader_descriptor_set.storage_buffers.end(),
                "Binding is already present!"
            );
			KB_CORE_ASSERT(
                shader_descriptor_set.image_samplers.find(binding) == shader_descriptor_set.image_samplers.end(),
                "Binding is already present!"
            );

			VkWriteDescriptorSet& vk_descriptor_set = shader_descriptor_set.write_descriptor_sets[image_sampler.name];
            vk_descriptor_set = {};
            vk_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
            vk_descriptor_set.descriptorType = layout_binding.descriptorType;
            vk_descriptor_set.descriptorCount = layout_binding.descriptorCount;
            vk_descriptor_set.dstBinding = layout_binding.binding;
		}

		VkDescriptorSetLayoutCreateInfo descriptor_layout_create_info = {};
		descriptor_layout_create_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
		descriptor_layout_create_info.pNext = nullptr;
		descriptor_layout_create_info.bindingCount = static_cast<uint32_t>(layout_bindings.size());
		descriptor_layout_create_info.pBindings = layout_bindings.data();

		KB_CORE_INFO("Vulkan creating descriptor set {0} with {1} ubo's, {2} ssbo's, {3} samplers and {4} storage images", set,
			shader_descriptor_set.uniform_buffers.size(),
			shader_descriptor_set.storage_buffers.size(),
			shader_descriptor_set.image_samplers.size(),
			shader_descriptor_set.storage_images.size()
		);

        // #TODO can we pre reserve the size?
		if (set >= m_descriptor_set_layouts.size())
			m_descriptor_set_layouts.resize(set + 1);

        if (vkCreateDescriptorSetLayout(
            device,
            &descriptor_layout_create_info,
            nullptr,
            &m_descriptor_set_layouts.at(set)
        ) != VK_SUCCESS)
		{
            KB_CORE_ASSERT(false, "Vulkan failed to create descriptor set layout!");
		}
	}
}

} // end namespace kb
