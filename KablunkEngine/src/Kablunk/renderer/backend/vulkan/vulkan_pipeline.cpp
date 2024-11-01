#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_pipeline.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_frame_buffer.h"

#include "Kablunk/renderer/render_command.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

namespace Utils
{
static VkPrimitiveTopology KbPrimitiveTopologyToVkPrimitiveTopology(primitive_topology_t topology)
{
	switch (topology)
	{
		case primitive_topology_t::points:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		case primitive_topology_t::lines:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		case primitive_topology_t::triangles:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		case primitive_topology_t::line_strip:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		case primitive_topology_t::triangle_strip:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		case primitive_topology_t::triangle_fan:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		default:								KB_CORE_ASSERT(false, "Unknown toplogy"); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
	}
}

static VkFormat KbShaderDataTypeToVulkanFormat(shader_data_type_t type)
{
	switch (type)
	{
	case shader_data_type_t::Int:		return VK_FORMAT_R32_SINT;
	case shader_data_type_t::Int2:		return VK_FORMAT_R32G32_SINT;
	case shader_data_type_t::Int3:		return VK_FORMAT_R32G32B32_SINT;
	case shader_data_type_t::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
	case shader_data_type_t::Float:     return VK_FORMAT_R32_SFLOAT;
	case shader_data_type_t::Float2:    return VK_FORMAT_R32G32_SFLOAT;
	case shader_data_type_t::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
	case shader_data_type_t::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
	default:						KB_CORE_ASSERT(false, "Unknown ShaderDataType"); return VK_FORMAT_UNDEFINED;
	}
}
}

vulkan_pipeline::vulkan_pipeline(const pipeline_specification_t& specification)
	: m_specification{ specification }
{
	KB_CORE_ASSERT(specification.shader, "no shader set!");
	KB_CORE_ASSERT(specification.m_target_frame_buffer, "no frame buffer set!");

    register_shader_dependency(specification.shader, arc<pipeline>{ this });

    vulkan_pipeline::invalidate();
}

vulkan_pipeline::~vulkan_pipeline()
{
	submit_resource_free([
        pipeline = m_vk_pipeline,
        pipeline_layout = m_vk_pipeline_layout,
        pipeline_cache = m_vk_pipeline_cache
    ]()
		{
			const auto vk_device = vulkan_context::get()->get_device()->get_vk_device();
			vkDestroyPipeline(vk_device, pipeline, nullptr);
            vkDestroyPipelineCache(vk_device, pipeline_cache, nullptr);
			vkDestroyPipelineLayout(vk_device, pipeline_layout, nullptr);
		}
	);
}

void vulkan_pipeline::invalidate()
{
    arc instance{ this };
	submit([instance]() mutable
		{
			instance->RT_Invalidate();
		});
}

void vulkan_pipeline::RT_Invalidate()
{
	VkDevice device = vulkan_context::get()->get_device()->get_vk_device();
	auto shader = m_specification.shader.As<vulkan_shader>();
    auto frame_buffer = m_specification.m_target_frame_buffer.As<vulkan_frame_buffer>();

	const auto& push_constant_ranges = shader->GetPushConstantRanges();

	std::vector<VkPushConstantRange> vulkan_push_constant_ranges(push_constant_ranges.size());
	for (uint32_t i = 0; i < push_constant_ranges.size(); i++)
	{
		const auto& push_constant_range = push_constant_ranges[i];
		VkPushConstantRange& vk_push_constant_range = vulkan_push_constant_ranges[i];

		vk_push_constant_range.stageFlags = push_constant_range.shader_stage;
		vk_push_constant_range.offset = push_constant_range.offset;
		vk_push_constant_range.size = push_constant_range.size;
	}

	// Create the pipeline layout that is used to generate the rendering pipelines that are based on this descriptor set layout
	// In a more complex scenario you would have different pipeline layouts for different descriptor set layouts that could be reused
    const auto descriptor_set_layouts = shader->GetAllDescriptorSetLayouts();

	VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
	pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipeline_layout_create_info.pNext = nullptr;
	pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
	pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
	pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(vulkan_push_constant_ranges.size());
	pipeline_layout_create_info.pPushConstantRanges = vulkan_push_constant_ranges.data();

	if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &m_vk_pipeline_layout) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create pipeline layout!");
#ifdef KB_DEBUG
    log::core::trace(
        log::logger_tag_t::pipeline,
        "[VulkanPipeline]: Created pipeline layout for '{}' with handle {}",
        m_specification.debug_name,
        static_cast<const void*>(&m_vk_pipeline_layout)
    );
#endif

	// Create the graphics pipeline used in this example
	// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
	// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
	// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

	VkGraphicsPipelineCreateInfo pipeline_create_info = {};
	pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
	pipeline_create_info.layout = m_vk_pipeline_layout;
	// Renderpass this pipeline is attached to
	pipeline_create_info.renderPass = frame_buffer->get_vk_render_pass();

	// Construct the different states making up the pipeline
	// Input assembly state describes how primitives are assembled
	// This pipeline will assemble vertex data as a triangle lists (though we only use one triangle)

	VkPipelineInputAssemblyStateCreateInfo input_assembly_state = {};
	input_assembly_state.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	input_assembly_state.topology = Utils::KbPrimitiveTopologyToVkPrimitiveTopology(m_specification.topology);

	// Rasterization state
	VkPipelineRasterizationStateCreateInfo raster_state = {};
	raster_state.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	raster_state.polygonMode = m_specification.wireframe ? VK_POLYGON_MODE_LINE : VK_POLYGON_MODE_FILL;
	raster_state.cullMode = m_specification.backface_culling ? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
	raster_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
	raster_state.depthClampEnable = VK_FALSE;
	raster_state.rasterizerDiscardEnable = VK_FALSE;
	raster_state.depthBiasEnable = VK_FALSE;
	// #TODO make dynamic when lines are implemented!
	raster_state.lineWidth = 1.0f; 

	// Color m_enable_blend state describes how m_enable_blend factors are calculated (if used)
	// We need one m_enable_blend attachment state per color attachment (even if blending is not used)

	size_t color_attachment_count = frame_buffer->get_color_attachment_count();
	std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_state(color_attachment_count);
	if (frame_buffer->get_specification().m_swap_chain_target)
	{
		blend_attachment_state[0].colorWriteMask = 0xf;
		blend_attachment_state[0].blendEnable = VK_TRUE;
		blend_attachment_state[0].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		blend_attachment_state[0].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		blend_attachment_state[0].colorBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_state[0].alphaBlendOp = VK_BLEND_OP_ADD;
		blend_attachment_state[0].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
		blend_attachment_state[0].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
	}
	else
	{
		for (size_t i = 0; i < color_attachment_count; i++)
		{
			if (!frame_buffer->get_specification().m_enable_blend)
				break;

			blend_attachment_state[i].colorWriteMask = 0xf;

			const auto& attachment_spec = frame_buffer->get_specification().m_attachments.Attachments[i];
			frame_buffer_blend_mode_t blend_mode = frame_buffer->get_specification().m_blend_mode == frame_buffer_blend_mode_t::none
				? attachment_spec.blend_mode
				: frame_buffer->get_specification().m_blend_mode;

			blend_attachment_state[i].blendEnable = attachment_spec.blend ? VK_TRUE : VK_FALSE;
			switch (blend_mode)
			{
				case frame_buffer_blend_mode_t::src_alpha_one_minus_src_alpha:
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					break;
				}
				case frame_buffer_blend_mode_t::one_zero:
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					break;
				}
				case frame_buffer_blend_mode_t::zero_src_color:
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
					break;
				}
				case frame_buffer_blend_mode_t::additive:
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					break;
				}
				default:
				{
					KB_CORE_ASSERT(false, "unknown frame_buffer_blend_mode_t!");
					break;
				}
			}

			blend_attachment_state[i].colorBlendOp = VK_BLEND_OP_ADD;
			blend_attachment_state[i].alphaBlendOp = VK_BLEND_OP_ADD;
		}
	}

	VkPipelineColorBlendStateCreateInfo color_blend_state = {};
	color_blend_state.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	color_blend_state.attachmentCount = static_cast<uint32_t>(blend_attachment_state.size());
	color_blend_state.pAttachments = blend_attachment_state.data();

	// Viewport state sets the number of viewports and scissor used in this pipeline
	// Note: This is actually overriden by the dynamic states (see below)

	VkPipelineViewportStateCreateInfo viewport_state = {};
	viewport_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewport_state.viewportCount = 1;
	viewport_state.scissorCount = 1;

	// Enable dynamic states
	// Most states are baked into the pipeline, but there are still a few dynamic states that can be changed within a command buffer
	// To be able to change these we need do specify which dynamic states will be changed using this pipeline. Their actual states are set later on in the command buffer.
	// For this example we will set the viewport and scissor using dynamic states

	std::vector<VkDynamicState> dynamic_state_enables;
	dynamic_state_enables.push_back(VK_DYNAMIC_STATE_VIEWPORT);
	dynamic_state_enables.push_back(VK_DYNAMIC_STATE_SCISSOR);
	if (m_specification.topology == primitive_topology_t::lines || m_specification.topology == primitive_topology_t::line_strip || m_specification.wireframe)
		dynamic_state_enables.push_back(VK_DYNAMIC_STATE_LINE_WIDTH);

	VkPipelineDynamicStateCreateInfo dynamic_state = {};
	dynamic_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamic_state.pDynamicStates = dynamic_state_enables.data();
	dynamic_state.dynamicStateCount = static_cast<uint32_t>(dynamic_state_enables.size());

	// Depth and stencil state containing depth and stencil compare and test operations
	// We only use depth tests and want depth tests and writes to be enabled and compare with less or equal
	VkPipelineDepthStencilStateCreateInfo depth_stencil_state = {};
	depth_stencil_state.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depth_stencil_state.depthTestEnable = m_specification.depth_test ? VK_TRUE : VK_FALSE;
	depth_stencil_state.depthWriteEnable = m_specification.depth_write ? VK_TRUE : VK_FALSE;
	depth_stencil_state.depthCompareOp = VK_COMPARE_OP_LESS_OR_EQUAL;
	depth_stencil_state.depthBoundsTestEnable = VK_FALSE;
	depth_stencil_state.back.failOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.passOp = VK_STENCIL_OP_KEEP;
	depth_stencil_state.back.compareOp = VK_COMPARE_OP_ALWAYS;
	depth_stencil_state.stencilTestEnable = VK_FALSE;
	depth_stencil_state.front = depth_stencil_state.back;

	// Multi sampling state
	// This example does not make use of multi sampling (for anti-aliasing), the state must still be set and passed to the pipeline
	VkPipelineMultisampleStateCreateInfo multisample_state = {};
	multisample_state.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisample_state.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
	multisample_state.pSampleMask = nullptr;

	// Vertex input descriptor
	buffer_layout& layout = m_specification.layout;
	buffer_layout& instance_layout = m_specification.instance_layout;

	std::vector<VkVertexInputBindingDescription> vertex_input_binding_desc;

	VkVertexInputBindingDescription& vertex_input_binding = vertex_input_binding_desc.emplace_back();
	vertex_input_binding.binding = 0;
	vertex_input_binding.stride = layout.get_stride();
	vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

	if (!instance_layout.get_elements().empty())
	{
		VkVertexInputBindingDescription& instance_input_binding = vertex_input_binding_desc.emplace_back();
		instance_input_binding.binding = 1;
		instance_input_binding.stride = instance_layout.get_stride();
		instance_input_binding.inputRate = VK_VERTEX_INPUT_RATE_INSTANCE;
	}

	// Input attribute bindings describe shader attribute locations and memory layouts
	std::vector<VkVertexInputAttributeDescription> vertex_input_attributes(layout.get_elements().size() + instance_layout.get_elements().size());

	uint32_t location = 0;
	for (const auto& element : layout)
	{
		vertex_input_attributes[location].binding = 0;
		vertex_input_attributes[location].location = location;
		vertex_input_attributes[location].format = Utils::KbShaderDataTypeToVulkanFormat(element.Type);
		vertex_input_attributes[location].offset = static_cast<uint32_t>(element.Offset);
		location++;
	}

	for (const auto& element : instance_layout)
	{
		vertex_input_attributes[location].binding = 1;
		vertex_input_attributes[location].location = location;
		vertex_input_attributes[location].format = Utils::KbShaderDataTypeToVulkanFormat(element.Type);
		vertex_input_attributes[location].offset = static_cast<uint32_t>(element.Offset);
		location++;
	}

	// Vertex input state used for pipeline creation
	VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
	vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_binding_desc.size());
	vertex_input_state.pVertexBindingDescriptions = vertex_input_binding_desc.data();
	vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size());
	vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

	const auto& shader_stages = shader->GetPipelineShaderStageCreateInfos();

	// Set pipeline shader stage info
	pipeline_create_info.stageCount = static_cast<uint32_t>(shader_stages.size());
	pipeline_create_info.pStages = shader_stages.data();

	// Assign the pipeline states to the pipeline creation info structure
	pipeline_create_info.pVertexInputState = &vertex_input_state;
	pipeline_create_info.pInputAssemblyState = &input_assembly_state;
	pipeline_create_info.pRasterizationState = &raster_state;
	pipeline_create_info.pColorBlendState = &color_blend_state;
	pipeline_create_info.pMultisampleState = &multisample_state;
	pipeline_create_info.pViewportState = &viewport_state;
	pipeline_create_info.pDepthStencilState = &depth_stencil_state;
	pipeline_create_info.renderPass = frame_buffer->get_vk_render_pass();
	pipeline_create_info.pDynamicState = &dynamic_state;

	// what is a pipeline cache?
	VkPipelineCacheCreateInfo pipeline_cache_create_info = {};
	pipeline_cache_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
	if (vkCreatePipelineCache(device, &pipeline_cache_create_info, nullptr, &m_vk_pipeline_cache) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create Pipeline Cache!");

	// Create rendering pipeline using the specified states
	if (vkCreateGraphicsPipelines(device, m_vk_pipeline_cache, 1, &pipeline_create_info, nullptr, &m_vk_pipeline) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to create pipeline!");

#ifdef KB_DEBUG
    log::core::trace(
        log::logger_tag_t::pipeline,
        "[VulkanPipeline]: Created pipeline '{}' with handle {}",
        m_specification.debug_name,
        static_cast<const void*>(&m_vk_pipeline)
    );
#endif
}

} // end namespace kb::render::backend::vk
