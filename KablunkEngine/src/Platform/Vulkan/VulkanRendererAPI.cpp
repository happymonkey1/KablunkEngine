#include "kablunkpch.h"

#include "Kablunk/Renderer/Renderer.h"
#include "Platform/Vulkan/VulkanRendererAPI.h"

#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/VulkanImage.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/VulkanFramebuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanStorageBuffer.h"
#include "Platform/Vulkan/VulkanUniformBuffer.h"
#include "Platform/Vulkan/VulkanStorageBufferSet.h"
#include "Platform/Vulkan/VulkanUniformBufferSet.h"

#include "Kablunk/Renderer/SceneRenderer.h"
#include "Kablunk/Renderer/RenderCommand.h"

#include "imgui.h"
#include "backends/imgui_impl_glfw.h"
#include "backends/imgui_impl_vulkan.h"

#include "Kablunk/Core/Application.h"

#include <vulkan/vulkan.h>

namespace kb
{

struct VulkanRendererData
{
	ref<VertexBuffer> quad_vertex_buffer;
	ref<IndexBuffer> quad_index_buffer;

	std::vector<VkDescriptorPool> descriptor_pools;
	std::vector<uint32_t> descriptor_pool_allocation_count;

	kb::unordered_flat_map<UniformBufferSet*, kb::unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> uniform_buffer_write_descriptor_cache;
	kb::unordered_flat_map<StorageBufferSet*, kb::unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> storage_buffer_write_descriptor_cache;

	int32_t draw_call_count = 0;
};

static VulkanRendererData* s_renderer_data = nullptr;



VulkanRendererAPI::~VulkanRendererAPI()
{
	
}

void VulkanRendererAPI::Init()
{
    KB_PROFILE_SCOPE;

	//Renderer::GetShaderLibrary()->Load("resources/shaders/Kablunk_pbr_static.glsl");

	s_renderer_data = new VulkanRendererData{};

	s_renderer_data->descriptor_pools.resize(render::get_frames_in_flights());
	s_renderer_data->descriptor_pool_allocation_count.resize(render::get_frames_in_flights());

	// Create descriptor pools
	render::submit([]() mutable
		{
			// Create Descriptor Pool
			VkDescriptorPoolSize pool_sizes[] =
			{
				{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
				{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
				{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
				{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
			};

			VkDescriptorPoolCreateInfo pool_info = {};
			pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
			pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
			pool_info.maxSets = 100000;
			// #TODO Why are we using IM_ARRAYSIZE here?
			pool_info.poolSizeCount = (uint32_t)IM_ARRAYSIZE(pool_sizes);
			pool_info.pPoolSizes = pool_sizes;
			VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
			uint32_t frames_in_flight = render::get_frames_in_flights();
			for (uint32_t i = 0; i < frames_in_flight; i++)
			{
				if (vkCreateDescriptorPool(vk_device, &pool_info, nullptr, &s_renderer_data->descriptor_pools[i]) != VK_SUCCESS)
					KB_CORE_ASSERT(false, "failed to create descriptor pools!");
				s_renderer_data->descriptor_pool_allocation_count[i] = 0;
			}
		});

	// Create fullscreen quad
	float x = -1;
	float y = -1;
	float width = 2, height = 2;
	struct QuadVertex
	{
		vec3_packed Position;
		vec2_packed TexCoord;
	};

	QuadVertex* data = new QuadVertex[4];

	data[0].Position = glm::vec3(x, y, 0.0f);
	data[0].TexCoord = glm::vec2(0, 0);

	data[1].Position = glm::vec3(x + width, y, 0.0f);
	data[1].TexCoord = glm::vec2(1, 0);

	data[2].Position = glm::vec3(x + width, y + height, 0.0f);
	data[2].TexCoord = glm::vec2(1, 1);

	data[3].Position = glm::vec3(x, y + height, 0.0f);
	data[3].TexCoord = glm::vec2(0, 1);

	KB_CORE_INFO("creating fullscreen quad vertex buffer!");
	s_renderer_data->quad_vertex_buffer = VertexBuffer::Create(data, 4 * sizeof(QuadVertex));
    const uint32_t indices[6] = { 0, 1, 2, 2, 3, 0, };
	KB_CORE_INFO("creating fullscreen quad index buffer!");
	s_renderer_data->quad_index_buffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
}

void VulkanRendererAPI::Shutdown()
{
    KB_PROFILE_SCOPE;
	KB_CORE_INFO("Shutting down VulkanRenderer!");
    const VkDevice device = VulkanContext::Get()->GetDevice()->GetVkDevice();
	vkDeviceWaitIdle(device);

	VulkanShader::ClearUniformBuffers();

    for (const auto& descriptor_pool : s_renderer_data->descriptor_pools)
    {
        KB_CORE_INFO("[VulkanRendererApi]: destroying descriptor pool {}", static_cast<void*>(descriptor_pool));
        vkDestroyDescriptorPool(device, descriptor_pool, nullptr);
    }

    render::get_render_command_queue().execute();

	for (u32 i = 0; i < render::get_frames_in_flights(); ++i)
	{
		auto& queue = render::get_render_resource_release_queue(i);
		queue.execute();
	}

	delete s_renderer_data;

	s_renderer_data = nullptr;
}

void VulkanRendererAPI::BeginFrame()
{
    KB_PROFILE_SCOPE;
	render::submit([]()
		{
            KB_PROFILE_SCOPE;

            const VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();

			// Reset descriptor pools here
            const VkDevice vk_device = VulkanContext::Get()->GetDevice()->GetVkDevice();
            const u32 buffer_index = swap_chain.GetCurrentBufferIndex();
			vkResetDescriptorPool(vk_device, s_renderer_data->descriptor_pools[buffer_index], 0);
			memset(
                s_renderer_data->descriptor_pool_allocation_count.data(),
                0,
                s_renderer_data->descriptor_pool_allocation_count.size() * sizeof(u32)
            );
		});
}

void VulkanRendererAPI::EndFrame()
{

}

void VulkanRendererAPI::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
{

}

void VulkanRendererAPI::SetClearColor(const glm::vec4& color)
{

}

void VulkanRendererAPI::Clear()
{

}

void VulkanRendererAPI::DrawIndexed(const ref<VertexArray>& vertexArray, uint32_t indexCount /*= 0*/)
{

}

void VulkanRendererAPI::ClearImage(ref<RenderCommandBuffer> command_buffer, ref<Image2D> image)
{
    KB_PROFILE_SCOPE;
	render::submit([command_buffer, image = image.As<VulkanImage2D>()]
		{
            KB_PROFILE_SCOPE;
            
			const auto vk_command_buffer = command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(render::rt_get_current_frame_index());
			VkImageSubresourceRange subresource_range{};
			subresource_range.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
			subresource_range.baseMipLevel = 0;
			subresource_range.levelCount = image->GetSpecification().mips;
			subresource_range.layerCount = image->GetSpecification().layers;

            constexpr VkClearColorValue clear_color{ 0.f, 0.f, 0.f, 0.f };
			vkCmdClearColorImage(vk_command_buffer, image->GetImageInfo().image, image->GetSpecification().usage == ImageUsage::Storage ? VK_IMAGE_LAYOUT_GENERAL : VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL, &clear_color, 1, &subresource_range);
		});
}

void VulkanRendererAPI::RenderMesh(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<MaterialTable> material_table, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count)
{
    KB_PROFILE_SCOPE;

	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, mesh, submesh_index, material_table, transform_buffer, transform_offset, instance_count]()
		{
            KB_PROFILE_SCOPE;

			uint32_t frame_index = render::rt_get_current_frame_index();
			VkCommandBuffer vk_command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			// retrieve mesh data vertex buffer and bind
			ref<MeshData> mesh_data = mesh->GetMeshData();
			ref<VulkanVertexBuffer> vertex_buffer = mesh_data->GetVertexBuffer().As<VulkanVertexBuffer>();
			VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();
			VkDeviceSize vertex_offsets[1] = { 0 };
			vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, vertex_offsets);

			// retrieve mesh transform vertex buffer and bind
			ref<VulkanVertexBuffer> vulkan_transform_buffer = transform_buffer.As<VulkanVertexBuffer>();
			VkBuffer vk_transform_buffer = vulkan_transform_buffer->GetVkBuffer();
			VkDeviceSize transform_offsets[1] = { transform_offset };
			vkCmdBindVertexBuffers(vk_command_buffer, 1, 1, &vk_transform_buffer, transform_offsets);

			ref<VulkanIndexBuffer> index_buffer = mesh_data->GetIndexBuffer().As<VulkanIndexBuffer>();
			VkBuffer vk_index_buffer = index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

			const auto& mesh_asset_submeshes = mesh_data->GetSubmeshes();
			const Submesh& submesh = mesh_asset_submeshes[submesh_index];
			const auto& mesh_material_table = mesh->GetMaterials();
			uint32_t material_count = mesh_material_table->GetMaterialCount();
			ref<MaterialAsset> material = material_table->HasMaterial(submesh.Material_index) ? 
				material_table->GetMaterial(submesh.Material_index) : mesh_material_table->GetMaterial(submesh.Material_index);
			ref<VulkanMaterial> vulkan_material = material->GetMaterial().As<VulkanMaterial>();
			RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();
			VkPipeline pipeline = vulkan_pipeline->GetVkPipeline();
			VkPipelineLayout pipeline_layout = vulkan_pipeline->GetVkPipelineLayout();
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			// #TODO line width

			VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(frame_index);
			if (descriptor_set)
				vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

			owning_buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
			if (uniform_storage_buffer)
			{
				vkCmdPushConstants(
					vk_command_buffer, 
					pipeline_layout, 
					VK_SHADER_STAGE_FRAGMENT_BIT, 
					0, 
					static_cast<uint32_t>(uniform_storage_buffer.size()), 
					uniform_storage_buffer.get()
				);
			}

			// #TODO submeshes

			// #TODO INSTANCING
			vkCmdDrawIndexed(
				vk_command_buffer, 
				submesh.IndexCount, 
				instance_count, 
				submesh.BaseIndex, 
				submesh.BaseVertex, 
				0
			);

			//push_constant_buffer.Release();
		}
	);
}

void VulkanRendererAPI::RenderMeshWithMaterial(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Mesh> mesh, uint32_t submesh_index, ref<Material> material, ref<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, owning_buffer additional_uniforms)
{
    KB_PROFILE_SCOPE;

	KB_CORE_ASSERT(mesh, "Mesh is null!");
	KB_CORE_ASSERT(mesh->GetMeshData(), "MeshData is null!");

	// Why are we copying the uniform buffer to a new one?
	owning_buffer push_constant_buffer;
	if (additional_uniforms.size() > 0)
	{
		push_constant_buffer.Allocate(additional_uniforms.size());
		push_constant_buffer.Write(additional_uniforms.get(), additional_uniforms.size());
	}

	ref<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, mesh, submesh_index, vulkan_material, transform_buffer, transform_offset, instance_count, push_constant_buffer]()
		{
            KB_PROFILE_SCOPE;

			uint32_t frame_index = render::rt_get_current_frame_index();
			VkCommandBuffer vk_command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			ref<MeshData> mesh_data = mesh->GetMeshData();
			ref<VulkanVertexBuffer> vertex_buffer = mesh_data->GetVertexBuffer().As<VulkanVertexBuffer>();
			VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();
			VkDeviceSize vertex_offsets[1] = { 0 };
			vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, vertex_offsets);

			ref<VulkanVertexBuffer> vulkan_transform_buffer = transform_buffer.As<VulkanVertexBuffer>();
			VkBuffer vk_transform_buffer = vulkan_transform_buffer->GetVkBuffer();
			VkDeviceSize transform_offsets[1] = { 0 };
			vkCmdBindVertexBuffers(vk_command_buffer, 1, 1, &vk_transform_buffer, transform_offsets);

			ref<VulkanIndexBuffer> index_buffer = mesh_data->GetIndexBuffer().As<VulkanIndexBuffer>();
			VkBuffer vk_index_buffer = index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

			RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();
			VkPipeline pipeline = vulkan_pipeline->GetVkPipeline();
			VkPipelineLayout pipeline_layout = vulkan_pipeline->GetVkPipelineLayout();
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			// #TODO line width

			VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(frame_index);
			if (descriptor_set)
				vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

			owning_buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
			if (uniform_storage_buffer)
				vkCmdPushConstants(vk_command_buffer, pipeline_layout, VK_SHADER_STAGE_FRAGMENT_BIT, 0, static_cast<uint32_t>(uniform_storage_buffer.size()), uniform_storage_buffer.get());

			// #TODO submeshes

			if (push_constant_buffer.size() > 0)
			{
				vkCmdPushConstants(
					vk_command_buffer, 
					pipeline_layout, 
					VK_SHADER_STAGE_VERTEX_BIT, 
					0, 
					static_cast<uint32_t>(push_constant_buffer.size()), 
					push_constant_buffer.get()
				);
			}

			vkCmdDrawIndexed(vk_command_buffer, static_cast<uint32_t>(mesh_data->GetIndicies().size()), instance_count, 0, 0, 0);

			//push_constant_buffer.Release();
		}
	);
}

void VulkanRendererAPI::render_instanced_submesh(
	ref<RenderCommandBuffer> render_command_buffer,
	ref<Pipeline> pipeline,
	ref<UniformBufferSet> uniform_buffer_set,
	ref<StorageBufferSet> storage_buffer_set,
	ref<Mesh> mesh,
	uint32_t submesh_index,
	ref<MaterialTable> material_table,
	ref<VertexBuffer> transform_buffer,
	uint32_t transform_offset,
	uint32_t instance_count
)
{
    KB_PROFILE_SCOPE;

	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, mesh, submesh_index, material_table, transform_buffer, transform_offset, instance_count]()
		{
            KB_PROFILE_SCOPE;

			uint32_t frame_index = render::rt_get_current_frame_index();
			VkCommandBuffer vk_command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			// retrieve mesh data vertex buffer and bind
			ref<MeshData> mesh_data = mesh->GetMeshData();
			ref<VulkanVertexBuffer> vertex_buffer = mesh_data->GetVertexBuffer().As<VulkanVertexBuffer>();
			VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();
			VkDeviceSize vertex_offsets[1] = { 0 };
			vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, vertex_offsets);

			// retrieve mesh transform vertex buffer and bind
			ref<VulkanVertexBuffer> vulkan_transform_buffer = transform_buffer.As<VulkanVertexBuffer>();
			VkBuffer vk_transform_buffer = vulkan_transform_buffer->GetVkBuffer();
			VkDeviceSize transform_offsets[1] = { transform_offset };
			vkCmdBindVertexBuffers(vk_command_buffer, 1, 1, &vk_transform_buffer, transform_offsets);

			ref<VulkanIndexBuffer> index_buffer = mesh_data->GetIndexBuffer().As<VulkanIndexBuffer>();
			VkBuffer vk_index_buffer = index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

			const auto& mesh_asset_submeshes = mesh_data->GetSubmeshes();
			const Submesh& submesh = mesh_asset_submeshes[submesh_index];
			const auto& mesh_material_table = mesh->GetMaterials();
			uint32_t material_count = mesh_material_table->GetMaterialCount();
			ref<MaterialAsset> material = material_table->HasMaterial(submesh.Material_index) ? material_table->GetMaterial(submesh.Material_index) : mesh_material_table->GetMaterial(submesh.Material_index);
			ref<VulkanMaterial> vulkan_material = material->GetMaterial().As<VulkanMaterial>();
			RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();
			VkPipeline pipeline = vulkan_pipeline->GetVkPipeline();
			VkPipelineLayout pipeline_layout = vulkan_pipeline->GetVkPipelineLayout();
			vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

			// #TODO line width

			VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(frame_index);
			if (descriptor_set)
				vkCmdBindDescriptorSets(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline_layout, 0, 1, &descriptor_set, 0, nullptr);

			owning_buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
			if (uniform_storage_buffer)
			{
				vkCmdPushConstants(
					vk_command_buffer,
					pipeline_layout,
					VK_SHADER_STAGE_FRAGMENT_BIT,
					0,
					static_cast<uint32_t>(uniform_storage_buffer.size()),
					uniform_storage_buffer.get()
				);
			}

			vkCmdDrawIndexed(
				vk_command_buffer,
				submesh.IndexCount,
				instance_count,
				submesh.BaseIndex,
				submesh.BaseVertex,
				0
			);
		}
	);
}

void VulkanRendererAPI::SubmitFullscreenQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material)
{
    KB_PROFILE_SCOPE;

	ref<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, vulkan_material]() mutable
		{
            KB_PROFILE_SCOPE;

			const u32 frame_index = render::rt_get_current_frame_index();
            const VkCommandBuffer command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

            const VkPipelineLayout layout = vulkan_pipeline->GetVkPipelineLayout();

			auto vulkan_vertex_buffer = s_renderer_data->quad_vertex_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 };
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

			auto vulkan_index_buffer = s_renderer_data->quad_index_buffer.As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = vulkan_index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkPipeline vk_pipeline = vulkan_pipeline->GetVkPipeline();
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

			//uint32_t frame_index = render::rt_get_current_frame_index();
            const VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(frame_index);
			if (descriptor_set)
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_set, 0, nullptr);

			owning_buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
			if (uniform_storage_buffer.size())
			{
                vkCmdPushConstants(
                    command_buffer,
                    layout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    0,
                    static_cast<u32>(uniform_storage_buffer.size()),
                    uniform_storage_buffer.get()
                );
			}

			vkCmdDrawIndexed(command_buffer, s_renderer_data->quad_index_buffer->GetCount(), 1, 0, 0, 0);
		});

}

void VulkanRendererAPI::RenderQuad(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, const glm::mat4& transform)
{
    KB_PROFILE_SCOPE;

	ref<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, vulkan_material, transform]() mutable
		{
            KB_PROFILE_SCOPE;

            const u32 frame_index = render::rt_get_current_frame_index();
            const VkCommandBuffer vk_cmd_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

            const VkPipelineLayout vk_pipeline_layout = vulkan_pipeline->GetVkPipelineLayout();

			ref<VulkanVertexBuffer> vulkan_vertex_buffer = s_renderer_data->quad_vertex_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
			// WTF is this for?
            constexpr VkDeviceSize offsets[1] = { 0 };

			vkCmdBindVertexBuffers(vk_cmd_buffer, 0, 1, &vk_vertex_buffer, offsets);

			ref<VulkanIndexBuffer> vulkan_index_buffer = s_renderer_data->quad_index_buffer.As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = vulkan_index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(vk_cmd_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkPipeline vk_pipeline = vulkan_pipeline->GetVkPipeline();
			vkCmdBindPipeline(vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			RT_UpdateMaterialForRendering(vulkan_material, uniform_buffer_set, storage_buffer_set);

            const VkDescriptorSet vk_descriptor_set = vulkan_material->GetDescriptorSet(frame_index);
			if (vk_descriptor_set)
				vkCmdBindDescriptorSets(vk_cmd_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline_layout, 0, 1, &vk_descriptor_set, 0, nullptr);

			owning_buffer uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();

			vkCmdPushConstants(vk_cmd_buffer, vk_pipeline_layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);
			vkCmdPushConstants(
				vk_cmd_buffer,
				vk_pipeline_layout,
				VK_SHADER_STAGE_FRAGMENT_BIT,
				sizeof(glm::mat4),
				static_cast<u32>(uniform_storage_buffer.size()),
				uniform_storage_buffer.get()
			);
			vkCmdDrawIndexed(vk_cmd_buffer, s_renderer_data->quad_index_buffer->GetCount(), 1, 0, 0, 0);
		});
}

void VulkanRendererAPI::RenderGeometry(ref<RenderCommandBuffer> render_command_buffer, ref<Pipeline> pipeline, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set, ref<Material> material, ref<VertexBuffer> vertex_buffer, ref<IndexBuffer> index_buffer, const glm::mat4& transform, uint32_t index_count /*= 0*/)
{
    KB_PROFILE_SCOPE;

	ref<VulkanMaterial> vulkan_material = material.As<VulkanMaterial>();
	if (index_count == 0)
		index_count = index_buffer->GetCount();

	render::submit([render_command_buffer, pipeline, uniform_buffer_set, storage_buffer_set, vulkan_material, vertex_buffer, index_buffer, transform, index_count]() mutable
		{
            KB_PROFILE_SCOPE;

            const u32 frameIndex = render::rt_get_current_frame_index();
            const VkCommandBuffer command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frameIndex);

			ref<VulkanPipeline> vulkan_pipeline = pipeline.As<VulkanPipeline>();

            const VkPipelineLayout layout = vulkan_pipeline->GetVkPipelineLayout();

			auto vulkan_geometry_vertex_buffer = vertex_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vulkan_geometry_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 }; // wtf is this
			vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

			auto vulkan_geometry_index_buffer = index_buffer.As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = vulkan_geometry_index_buffer->GetVkBuffer();
			vkCmdBindIndexBuffer(command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkPipeline vk_pipeline = vulkan_pipeline->GetVkPipeline();
			vkCmdBindPipeline(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

			const auto& write_descriptors = RT_RetrieveOrCreateUniformBufferWriteDescriptors(uniform_buffer_set, vulkan_material);
			vulkan_material->RT_UpdateForRendering(write_descriptors);

            const VkDescriptorSet descriptor_set = vulkan_material->GetDescriptorSet(frameIndex);
			if (descriptor_set)
				vkCmdBindDescriptorSets(command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, layout, 0, 1, &descriptor_set, 0, nullptr);

			vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(glm::mat4), &transform);

			const owning_buffer& uniform_storage_buffer = vulkan_material->GetUniformStorageBuffer();
			if (uniform_storage_buffer)
				vkCmdPushConstants(command_buffer, layout, VK_SHADER_STAGE_FRAGMENT_BIT, sizeof(glm::mat4), static_cast<u32>(uniform_storage_buffer.size()), uniform_storage_buffer.get());

			vkCmdDrawIndexed(command_buffer, index_count, 1, 0, 0, 0);
		});
}

void VulkanRendererAPI::SetLineWidth(ref<RenderCommandBuffer> render_command_buffer, float line_width)
{
	render::submit([width = line_width, render_cmd_buffer = render_command_buffer]()
	{
		const u32 frame_index = render::rt_get_current_frame_index();
        const VkCommandBuffer vk_cmd_buffer = render_cmd_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);
		vkCmdSetLineWidth(vk_cmd_buffer, width);
	});
}

void VulkanRendererAPI::WaitAndRender()
{
	KB_CORE_ASSERT(false, "deprecated")
	render::get_render_command_queue().execute();
}

const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateUniformBufferWriteDescriptors(ref<UniformBufferSet> uniform_buffer_set, ref<VulkanMaterial> material)
{
    KB_PROFILE_SCOPE;

    const size_t shader_hash = material->GetShader()->GetHash();
	if (s_renderer_data->uniform_buffer_write_descriptor_cache.find(uniform_buffer_set.get()) != s_renderer_data->uniform_buffer_write_descriptor_cache.end())
	{
		const auto& shader_map = s_renderer_data->uniform_buffer_write_descriptor_cache.at(uniform_buffer_set.get());
		if (shader_map.find(shader_hash) != shader_map.end())
		{
			const auto& write_descriptors = shader_map.at(shader_hash);
			return write_descriptors;
		}
	}

	const u32 frames_in_flight = render::get_frames_in_flights();
	ref<VulkanShader> shader = material->GetShader().As<VulkanShader>();
	if (shader->HasDescriptorSet(0))
	{
		const auto& shader_descriptor_sets = shader->GetShaderDescriptorSets();
		if (!shader_descriptor_sets.empty())
		{
			for (auto&& [binding, shader_uniform_buffer] : shader_descriptor_sets[0].uniform_buffers)
			{
				auto& write_descriptors = s_renderer_data->uniform_buffer_write_descriptor_cache[uniform_buffer_set.get()][shader_hash];
				write_descriptors.resize(frames_in_flight);
				for (u32 frame = 0; frame < frames_in_flight; ++frame)
				{
					// set = 0 for now
					ref uniform_buffer = uniform_buffer_set->Get(binding, 0, frame).As<VulkanUniformBuffer>();

					VkWriteDescriptorSet write_descriptor_set = {};
					write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_descriptor_set.descriptorCount = 1;
					write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
					write_descriptor_set.pBufferInfo = &uniform_buffer->GetDescriptorBufferInfo();
					write_descriptor_set.dstBinding = uniform_buffer->GetBinding();
					write_descriptors[frame].push_back(write_descriptor_set);
				}
			}
		}
	}

	return s_renderer_data->uniform_buffer_write_descriptor_cache[uniform_buffer_set.get()][shader_hash];
}

const std::vector<std::vector<VkWriteDescriptorSet>>& VulkanRendererAPI::RT_RetrieveOrCreateStorageBufferWriteDescriptors(ref<StorageBufferSet> storage_buffer_set, ref<VulkanMaterial> material)
{
    KB_PROFILE_SCOPE;

    const size_t shader_hash = material->GetShader()->GetHash();
	if (s_renderer_data->storage_buffer_write_descriptor_cache.find(storage_buffer_set.get()) != s_renderer_data->storage_buffer_write_descriptor_cache.end())
	{
		const auto& shader_map = s_renderer_data->storage_buffer_write_descriptor_cache.at(storage_buffer_set.get());
		if (shader_map.find(shader_hash) != shader_map.end())
		{
			const auto& write_descriptors = shader_map.at(shader_hash);
			return write_descriptors;
		}
	}

	const u32 frames_in_flight = render::get_frames_in_flights();
	ref<VulkanShader> shader = material->GetShader().As<VulkanShader>();
	if (shader->HasDescriptorSet(0))
	{
		const auto& shader_descriptor_set = shader->GetShaderDescriptorSets();
		if (!shader_descriptor_set.empty())
		{
			for (auto&& [binding, shader_storage_buffer] : shader_descriptor_set[0].storage_buffers)
			{
				auto& write_descriptor = s_renderer_data->storage_buffer_write_descriptor_cache[storage_buffer_set.get()][shader_hash];
				write_descriptor.resize(frames_in_flight);
				for (u32 frame = 0; frame < frames_in_flight; ++frame)
				{
					ref storage_buffer = storage_buffer_set->Get(binding, 0, frame).As<VulkanStorageBuffer>(); // set = 0 for now

					VkWriteDescriptorSet write_descriptor_set = {};
					write_descriptor_set.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
					write_descriptor_set.descriptorCount = 1;
					write_descriptor_set.descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
					write_descriptor_set.pBufferInfo = &storage_buffer->GetVkDescriptorInfo();
					write_descriptor_set.dstBinding = storage_buffer->GetBinding();
					write_descriptor[frame].push_back(write_descriptor_set);
				}
			}
		}
	}

	return s_renderer_data->storage_buffer_write_descriptor_cache[storage_buffer_set.get()][shader_hash];
}

void VulkanRendererAPI::RT_UpdateMaterialForRendering(ref<VulkanMaterial> vulkan_material, ref<UniformBufferSet> uniform_buffer_set, ref<StorageBufferSet> storage_buffer_set)
{
    KB_PROFILE_SCOPE;

	if (uniform_buffer_set)
	{
		auto write_description = RT_RetrieveOrCreateUniformBufferWriteDescriptors(uniform_buffer_set, vulkan_material);
		if (storage_buffer_set)
		{
			const auto& storage_buffer_write_descriptors = RT_RetrieveOrCreateStorageBufferWriteDescriptors(storage_buffer_set, vulkan_material);

			const uint32_t frames_in_flight = render::get_frames_in_flights();
			for (uint32_t frame = 0; frame < frames_in_flight; frame++)
			{
				write_description[frame].reserve(write_description[frame].size() + storage_buffer_write_descriptors[frame].size());
				write_description[frame].insert(write_description[frame].end(), storage_buffer_write_descriptors[frame].begin(), storage_buffer_write_descriptors[frame].end());
			}
		}
		vulkan_material->RT_UpdateForRendering(write_description);
	}
	else
		vulkan_material->RT_UpdateForRendering();
}

VkDescriptorSet VulkanRendererAPI::RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info)
{
    KB_PROFILE_SCOPE;

    const auto device = VulkanContext::Get()->GetDevice()->GetVkDevice();
    u32 buffer_index = render::rt_get_current_frame_index();
	alloc_info.descriptorPool = s_renderer_data->descriptor_pools[buffer_index];
	
	VkDescriptorSet descriptor_set;
	if (vkAllocateDescriptorSets(device, &alloc_info, &descriptor_set) != VK_SUCCESS)
		KB_CORE_ASSERT(false, "Vulkan failed to allocate descriptor set!");

	s_renderer_data->descriptor_pool_allocation_count[buffer_index] += alloc_info.descriptorSetCount;
	return descriptor_set;
}

void VulkanRendererAPI::BeginRenderPass(ref<RenderCommandBuffer> render_command_buffer, const ref<RenderPass>& render_pass, bool explicit_clear)
{
    KB_PROFILE_SCOPE;

	render::submit([render_command_buffer, render_pass, explicit_clear]()
		{
            KB_PROFILE_SCOPE;

            const u32 frame_index = render::rt_get_current_frame_index();
            const VkCommandBuffer cmd_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

			const auto& framebuffer = render_pass->GetSpecification().target_framebuffer;
			ref<VulkanFramebuffer> vulkan_framebuffer = framebuffer.As<VulkanFramebuffer>();
			const auto& framebuffer_spec = vulkan_framebuffer->GetSpecification();

            u32 width = framebuffer_spec.width;
            u32 height = framebuffer_spec.height;

			VkViewport viewport{};
			viewport.minDepth = 0.0f;
			viewport.maxDepth = 1.0f;

			VkRenderPassBeginInfo render_pass_begin_info{};
			render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
			render_pass_begin_info.pNext = nullptr;
			render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
			render_pass_begin_info.renderArea.offset.x = 0;
			render_pass_begin_info.renderArea.offset.y = 0;
			render_pass_begin_info.renderArea.extent.width = width;
			render_pass_begin_info.renderArea.extent.height = height;
			if (framebuffer_spec.swap_chain_target)
			{
				VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();
				width = swap_chain.GetWidth();
				height = swap_chain.GetHeight();
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.pNext = nullptr;
				render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
				render_pass_begin_info.renderArea.offset.x = 0;
				render_pass_begin_info.renderArea.offset.y = 0;
				render_pass_begin_info.renderArea.extent.width = width;
				render_pass_begin_info.renderArea.extent.height = height;
				render_pass_begin_info.framebuffer = swap_chain.GetCurrentFramebuffer();

				viewport.x = 0.0f;
				viewport.y = static_cast<f32>(height);
				viewport.width = static_cast<f32>(width);
				viewport.height = -static_cast<f32>(height);
			}
			else
			{
				width = framebuffer->GetWidth();
				height = framebuffer->GetHeight();
				render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
				render_pass_begin_info.pNext = nullptr;
				render_pass_begin_info.renderPass = vulkan_framebuffer->GetVkRenderPass();
				render_pass_begin_info.renderArea.offset.x = 0;
				render_pass_begin_info.renderArea.offset.y = 0;
				render_pass_begin_info.renderArea.extent.width = width;
				render_pass_begin_info.renderArea.extent.height = height;
				render_pass_begin_info.framebuffer = vulkan_framebuffer->GetVkFramebuffer();

				viewport.x = 0.0f;
				viewport.y = 0.0f;
				viewport.width = static_cast<f32>(width);
				viewport.height = static_cast<f32>(height);
			}

			const auto& clear_values = vulkan_framebuffer->GetVkClearValues();
			render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
			render_pass_begin_info.pClearValues = clear_values.data();

			vkCmdBeginRenderPass(cmd_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

			if (explicit_clear)
			{
				const u32 color_attachment_count = static_cast<u32>(vulkan_framebuffer->GetColorAttachmentCount());
				const u32 total_attachment_count = color_attachment_count +
                    (vulkan_framebuffer->HasDepthAttachment() ? 1 : 0);
				KB_CORE_ASSERT(clear_values.size() == total_attachment_count, "uh oh");

				std::vector<VkClearAttachment> attachments(total_attachment_count);
				std::vector<VkClearRect> clear_rects(total_attachment_count);
				for (u32 i = 0; i < color_attachment_count; i++)
				{
					attachments[i].aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
					attachments[i].colorAttachment = i;
					attachments[i].clearValue = clear_values[i];

					clear_rects[i].rect.offset = { 0, 0 };
					clear_rects[i].rect.extent = { width, height };
					clear_rects[i].baseArrayLayer = 0;
					clear_rects[i].layerCount = 1;
				}

				if (vulkan_framebuffer->HasDepthAttachment())
				{
					attachments[color_attachment_count].aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
					attachments[color_attachment_count].clearValue = clear_values[color_attachment_count];
					clear_rects[color_attachment_count].rect.offset = { 0, 0 };
					clear_rects[color_attachment_count].rect.extent = { width, height };
					clear_rects[color_attachment_count].baseArrayLayer = 0;
					clear_rects[color_attachment_count].layerCount = 1;
				}

				vkCmdClearAttachments(
                    cmd_buffer,
                    total_attachment_count,
                    attachments.data(),
                    total_attachment_count,
                    clear_rects.data()
                );
			}

			// Update dynamic viewport state
			vkCmdSetViewport(cmd_buffer, 0, 1, &viewport);

			// Update dynamic scissor state
			VkRect2D scissor;
			scissor.extent.width = width;
			scissor.extent.height = height;
			scissor.offset.x = 0;
			scissor.offset.y = 0;
			vkCmdSetScissor(cmd_buffer, 0, 1, &scissor);
		});
}

void VulkanRendererAPI::EndRenderPass(ref<RenderCommandBuffer> render_command_buffer)
{
    KB_PROFILE_SCOPE;

	render::submit([render_command_buffer]()
		{
            KB_PROFILE_SCOPE;

            const VkCommandBuffer vk_command_buffer = render_command_buffer.As<VulkanRenderCommandBuffer>()->get_active_command_buffer();

			vkCmdEndRenderPass(vk_command_buffer);
		});
}
}
