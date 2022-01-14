#include "kablunkpch.h"

#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"

#include "Kablunk/Renderer/RenderCommand.h"

namespace Kablunk
{

	namespace Utils
	{
		static VkPrimitiveTopology KbPrimitiveTopologyToVkPrimitiveTopology(PrimitiveTopology topology)
		{
			switch (topology)
			{
				case PrimitiveTopology::Points:			return VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
				case PrimitiveTopology::Lines:			return VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
				case PrimitiveTopology::Triangles:		return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
				case PrimitiveTopology::LineStrip:		return VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
				case PrimitiveTopology::TriangleStrip:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
				case PrimitiveTopology::TriangleFan:	return VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
				default:								KB_CORE_ASSERT(false, "Unknown toplogy"); return VK_PRIMITIVE_TOPOLOGY_MAX_ENUM;
			}
		}

		static VkFormat KbShaderDataTypeToVulkanFormat(ShaderDataType type)
		{
			switch (type)
			{
			case ShaderDataType::Int:		return VK_FORMAT_R32_SINT;
			case ShaderDataType::Int2:		return VK_FORMAT_R32G32_SINT;
			case ShaderDataType::Int3:		return VK_FORMAT_R32G32B32_SINT;
			case ShaderDataType::Int4:		return VK_FORMAT_R32G32B32A32_SINT;
			case ShaderDataType::Float:     return VK_FORMAT_R32_SFLOAT;
			case ShaderDataType::Float2:    return VK_FORMAT_R32G32_SFLOAT;
			case ShaderDataType::Float3:    return VK_FORMAT_R32G32B32_SFLOAT;
			case ShaderDataType::Float4:    return VK_FORMAT_R32G32B32A32_SFLOAT;
			default:						KB_CORE_ASSERT(false, "Unknown ShaderDataType"); return VK_FORMAT_UNDEFINED;
			}
			
		}

	}

	VulkanPipeline::VulkanPipeline(const PipelineSpecification& specification)
		: m_specification{ specification }
	{
		KB_CORE_ASSERT(specification.shader, "no shader set!");
		KB_CORE_ASSERT(specification.render_pass, "no render pass set!");

		Invalidate();
	}

	VulkanPipeline::~VulkanPipeline()
	{
		KB_CORE_ERROR("Pipeline leaking!");
	}

	void VulkanPipeline::Invalidate()
	{
		IntrusiveRef<VulkanPipeline> instance = this;
		RenderCommand::Submit([instance]() mutable
			{
				instance->RT_Invalidate();
			});
	}

	void VulkanPipeline::RT_Invalidate()
	{
		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
		IntrusiveRef<VulkanShader> vulkan_shader = IntrusiveRef<VulkanShader>(m_specification.shader);
		IntrusiveRef<VulkanFramebuffer> framebuffer = m_specification.render_pass->GetSpecification().target_framebuffer.As<VulkanFramebuffer>();

		auto descriptor_set_layouts = vulkan_shader->GetAllDescriptorSetLayouts();

		const auto& push_constant_ranges = vulkan_shader->GetPushConstantRanges();

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

		VkPipelineLayoutCreateInfo pipeline_layout_create_info{};
		pipeline_layout_create_info.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
		pipeline_layout_create_info.pNext = nullptr;
		pipeline_layout_create_info.setLayoutCount = static_cast<uint32_t>(descriptor_set_layouts.size());
		pipeline_layout_create_info.pSetLayouts = descriptor_set_layouts.data();
		pipeline_layout_create_info.pushConstantRangeCount = static_cast<uint32_t>(vulkan_push_constant_ranges.size());
		pipeline_layout_create_info.pPushConstantRanges = vulkan_push_constant_ranges.data();

		if (vkCreatePipelineLayout(device, &pipeline_layout_create_info, nullptr, &m_vk_pipeline_layout) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create pipeline layout!");

		// Create the graphics pipeline used in this example
		// Vulkan uses the concept of rendering pipelines to encapsulate fixed states, replacing OpenGL's complex state machine
		// A pipeline is then stored and hashed on the GPU making pipeline changes very fast
		// Note: There are still a few dynamic states that are not directly part of the pipeline (but the info that they are used is)

		VkGraphicsPipelineCreateInfo pipeline_create_info = {};
		pipeline_create_info.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
		// The layout used for this pipeline (can be shared among multiple pipelines using the same layout)
		pipeline_create_info.layout = m_vk_pipeline_layout;
		// Renderpass this pipeline is attached to
		pipeline_create_info.renderPass = framebuffer->GetVkRenderPass();

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
		raster_state.cullMode = m_specification.backface_culling? VK_CULL_MODE_BACK_BIT : VK_CULL_MODE_NONE;
		raster_state.frontFace = VK_FRONT_FACE_CLOCKWISE;
		raster_state.depthClampEnable = VK_FALSE;
		raster_state.rasterizerDiscardEnable = VK_FALSE;
		raster_state.depthBiasEnable = VK_FALSE;
		raster_state.lineWidth = 1.0f; // #TODO make dynamic when lines are implemented!

		// Color blend state describes how blend factors are calculated (if used)
		// We need one blend attachment state per color attachment (even if blending is not used)

		size_t color_attachment_count = framebuffer->GetSpecification().swap_chain_target ? 1 : framebuffer->GetColorAttachmentCount();
		std::vector<VkPipelineColorBlendAttachmentState> blend_attachment_state(color_attachment_count);
		if (framebuffer->GetSpecification().swap_chain_target)
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
				if (!framebuffer->GetSpecification().blend)
					break;

				blend_attachment_state[i].colorWriteMask = 0xf;

				const auto& attachmentSpec = framebuffer->GetSpecification().Attachments.Attachments[i];
				FramebufferBlendMode blendMode = framebuffer->GetSpecification().blend_mode == FramebufferBlendMode::None
					? attachmentSpec.blend_mode
					: framebuffer->GetSpecification().blend_mode;

				blend_attachment_state[i].blendEnable = attachmentSpec.blend ? VK_TRUE : VK_FALSE;
				if (blendMode == FramebufferBlendMode::SrcAlphaOneMinusSrcAlpha)
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
				}
				else if (blendMode == FramebufferBlendMode::OneZero)
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_ONE;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
				}
				else if (blendMode == FramebufferBlendMode::Zero_SrcColor)
				{
					blend_attachment_state[i].srcColorBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].dstColorBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
					blend_attachment_state[i].srcAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
					blend_attachment_state[i].dstAlphaBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
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
		if (m_specification.topology == PrimitiveTopology::Lines || m_specification.topology == PrimitiveTopology::LineStrip || m_specification.wireframe)
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
		BufferLayout& layout = m_specification.layout;

		std::vector<VkVertexInputBindingDescription> vertex_input_binding_desc;

		VkVertexInputBindingDescription& vertex_input_binding = vertex_input_binding_desc.emplace_back();
		vertex_input_binding.binding = 0;
		vertex_input_binding.stride = layout.GetStride();
		vertex_input_binding.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;

		// Input attribute bindings describe shader attribute locations and memory layouts
		std::vector<VkVertexInputAttributeDescription> vertex_input_attributes(layout.GetElements().size());

		uint32_t location = 0;
		for (auto element : layout)
		{
			vertex_input_attributes[location].binding = 0;
			vertex_input_attributes[location].location = location;
			vertex_input_attributes[location].format = Utils::KbShaderDataTypeToVulkanFormat(element.Type);
			vertex_input_attributes[location].offset = element.Offset;
			location++;
		}

		// Vertex input state used for pipeline creation
		VkPipelineVertexInputStateCreateInfo vertex_input_state = {};
		vertex_input_state.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		vertex_input_state.vertexBindingDescriptionCount = static_cast<uint32_t>(vertex_input_binding_desc.size());
		vertex_input_state.pVertexBindingDescriptions = vertex_input_binding_desc.data();
		vertex_input_state.vertexAttributeDescriptionCount = static_cast<uint32_t>(vertex_input_attributes.size());
		vertex_input_state.pVertexAttributeDescriptions = vertex_input_attributes.data();

		const auto& shader_stages = vulkan_shader->GetPipelineShaderStageCreateInfos();

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
		pipeline_create_info.renderPass = framebuffer->GetVkRenderPass();
		pipeline_create_info.pDynamicState = &dynamic_state;

		// what is a pipeline cache?
		VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {};
		pipelineCacheCreateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO;
		VkPipelineCache pipelineCache;
		if (vkCreatePipelineCache(device, &pipelineCacheCreateInfo, nullptr, &pipelineCache) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create Pipeline Cache!");

		// Create rendering pipeline using the specified states
		if (vkCreateGraphicsPipelines(device, pipelineCache, 1, &pipeline_create_info, nullptr, &m_vk_pipeline) != VK_SUCCESS)
			KB_CORE_ASSERT(false, "Vulkan failed to create pipeline!");
	}

	void VulkanPipeline::SetUniformBuffer(IntrusiveRef<UniformBuffer> uniform_buffer, uint32_t binding, uint32_t set /*= 0*/)
	{
		IntrusiveRef<VulkanPipeline> instance = this;
		RenderCommand::Submit([instance, uniform_buffer, binding, set]() mutable
			{
				instance->RT_SetUniformBuffer(uniform_buffer, binding, set);
			});
	}

	void VulkanPipeline::RT_SetUniformBuffer(IntrusiveRef<UniformBuffer> uniform_buffer, uint32_t binding, uint32_t set /*= 0*/)
	{
		IntrusiveRef<VulkanUniformBuffer> vulkan_uniform_buffer = uniform_buffer.As<VulkanUniformBuffer>();

		KB_CORE_ASSERT(set < m_descriptor_sets.descriptor_sets.size(), "out of bounds!");

		VkWriteDescriptorSet write_descriptor_sets = {};
		write_descriptor_sets.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write_descriptor_sets.descriptorCount = 1;
		write_descriptor_sets.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write_descriptor_sets.pBufferInfo = &vulkan_uniform_buffer->GetDescriptorBufferInfo();
		write_descriptor_sets.dstBinding = binding;
		write_descriptor_sets.dstSet = m_descriptor_sets.descriptor_sets[set];

		KB_CORE_INFO("VulkanPipeline - Updating descriptor set (VulkanPipeline::SetUniformBuffer)");
		VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
		vkUpdateDescriptorSets(device, 1, &write_descriptor_sets, 0, nullptr);
	}


}
