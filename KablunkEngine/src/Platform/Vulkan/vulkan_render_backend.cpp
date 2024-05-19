#include "kablunkpch.h"

#include "Platform/Vulkan/vulkan_render_backend.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include "Platform/Vulkan/vulkan_core.h"
#include "Platform/Vulkan/vulkan_api.h"
#include "Platform/Vulkan/VulkanContext.h"
#include "Platform/Vulkan/vulkan_frame_buffer.h"
#include "Platform/Vulkan/VulkanIndexBuffer.h"
#include "Platform/Vulkan/VulkanMaterial.h"
#include "Platform/Vulkan/VulkanPipeline.h"
#include "Platform/Vulkan/VulkanRenderCommandBuffer.h"
#include "Platform/Vulkan/VulkanVertexBuffer.h"
#include "Platform/Vulkan/vulkan_render_pass.h"

#include <vulkan/vulkan.h>

namespace kb::render
{

struct vulkan_render_backend_data
{
    ref<VertexBuffer> m_quad_vertex_buffer{};
    ref<IndexBuffer> m_quad_index_buffer{};
    VulkanShader::ShaderMaterialDescriptorSet m_quad_descriptor_set{};

    VkDescriptorSet m_active_descriptor_set = nullptr;
    std::vector<VkDescriptorPool> m_descriptor_pools{};
    VkDescriptorPool m_material_descriptor_pool = nullptr;
    std::vector<uint32_t> m_descriptor_pool_allocation_count{};

    // UniformBufferSet -> Shader Hash -> Frame -> WriteDescriptor
    kb::unordered_flat_map<UniformBufferSet*, kb::unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> uniform_buffer_write_descriptor_cache{};
    // StorageBufferSet -> Shader Hash -> Frame -> WriteDescriptor
    kb::unordered_flat_map<StorageBufferSet*, kb::unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> storage_buffer_write_descriptor_cache{};

    int32_t draw_call_count = 0;
};

auto vulkan_render_backend::init() noexcept -> void
{
    m_renderer_data = std::make_unique<vulkan_render_backend_data>();
    const auto frames_in_flight = render::get_frames_in_flight();

    m_renderer_data->m_descriptor_pools.resize(frames_in_flight);
    m_renderer_data->m_descriptor_pool_allocation_count.resize(frames_in_flight);

    submit([frames_in_flight, this]() mutable
        {
            const VkDescriptorPoolSize vk_pool_sizes[] =
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

            const VkDescriptorPoolCreateInfo vk_pool_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = 100000,
                .poolSizeCount = sizeof(vk_pool_sizes) / sizeof(VkDescriptorPoolSize),
                .pPoolSizes = vk_pool_sizes
            };

            const auto vk_device = vk::get_current_vk_device();
            for (u32 i = 0; i < frames_in_flight; i++)
            {
                KB_VK_CHECK_RESULT(
                    vkCreateDescriptorPool(vk_device, &vk_pool_info, nullptr, &m_renderer_data->m_descriptor_pools[i])
                );
                m_renderer_data->m_descriptor_pool_allocation_count[i] = 0;
            }

            KB_VK_CHECK_RESULT(
                vkCreateDescriptorPool(vk_device, &vk_pool_info, nullptr, &m_renderer_data->m_material_descriptor_pool)
            );
        }
    );

    constexpr f32 x = -1.;
    constexpr f32 y = -1.;
    constexpr f32 width = 2.;
    constexpr f32 height = 2.;
    struct QuadVertex
    {
        vec3_packed m_position;
        vec2_packed m_tex_coord;
    };

    constexpr QuadVertex quad_data[4]
    {
        { .m_position = vec3_packed(x, y, 0.f), .m_tex_coord = vec2_packed(0, 0) },
        { .m_position = vec3_packed(x + width, y, 0.f), .m_tex_coord = vec2_packed(1, 0) },
        { .m_position = vec3_packed(x + width, y + height, 0.f), .m_tex_coord = vec2_packed(1, 1) },
        { .m_position = vec3_packed(x, y + height, 0.f), .m_tex_coord = vec2_packed(0, 1)}
    };

    log::core::info(
        log::logger_tag_t::renderer,
        "Creating fullscreen quad vertex buffer"
    );
    m_renderer_data->m_quad_vertex_buffer = VertexBuffer::Create(quad_data, 4 * sizeof(QuadVertex));
    constexpr u32 indices[6] = { 0, 1, 2, 2, 3, 0, };

    log::core::info(
        log::logger_tag_t::renderer,
        "Creating fullscreen quad index buffer"
    );
    m_renderer_data->m_quad_index_buffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
}

auto vulkan_render_backend::shutdown() noexcept -> void
{
    KB_PROFILE_SCOPE;
    log::core::info(
        log::logger_tag_t::renderer,
        "Shutting down Vulkan render backend"
    );
    vkDeviceWaitIdle(vk::get_current_vk_device());

    m_renderer_data.reset();
}

auto vulkan_render_backend::begin_frame() noexcept -> void
{
}

auto vulkan_render_backend::end_frame() noexcept -> void
{
}

auto vulkan_render_backend::begin_render_pass(
    const ref<RenderCommandBuffer>& p_render_command_buffer,
    const ref<render_pass>& p_render_pass,
    bool p_explicit_clear
) noexcept -> void
{
    submit([p_render_command_buffer, p_render_pass, p_explicit_clear]()
        {
            KB_PROFILE_SCOPE_NAMED("vulkan_render_backend::begin_render_pass");
            log::core::info(
                log::logger_tag_t::renderer,
                "vulkan_render_backend::begin_render_pass {}",
                p_render_pass->get_specification().m_debug_name
            );

            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer = p_render_command_buffer.As<VulkanRenderCommandBuffer>()->get_active_command_buffer();

            const auto frame_buffer = p_render_pass->get_target_frame_buffer();
            const auto vulkan_frame_buffer = frame_buffer.As<render::vulkan_frame_buffer>();
            const auto& frame_buffer_spec = vulkan_frame_buffer->get_specification();

            u32 width = frame_buffer_spec.m_width;
            u32 height = frame_buffer_spec.m_height;

            VkViewport viewport{};
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.pNext = nullptr;
            render_pass_begin_info.renderPass = vulkan_frame_buffer->GetVkRenderPass();
            render_pass_begin_info.renderArea.offset.x = 0;
            render_pass_begin_info.renderArea.offset.y = 0;
            render_pass_begin_info.renderArea.extent.width = width;
            render_pass_begin_info.renderArea.extent.height = height;
            if (frame_buffer_spec.m_swap_chain_target)
            {
                VulkanSwapChain& swap_chain = VulkanContext::Get()->GetSwapchain();
                width = swap_chain.GetWidth();
                height = swap_chain.GetHeight();
                render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_begin_info.pNext = nullptr;
                render_pass_begin_info.renderPass = vulkan_frame_buffer->GetVkRenderPass();
                render_pass_begin_info.renderArea.offset.x = 0;
                render_pass_begin_info.renderArea.offset.y = 0;
                render_pass_begin_info.renderArea.extent.width = width;
                render_pass_begin_info.renderArea.extent.height = height;
                render_pass_begin_info.framebuffer = swap_chain.GetCurrentFramebuffer();
                KB_CORE_ASSERT(render_pass_begin_info.framebuffer, "render pass swap chain framebuffer is null?");

                viewport.x = 0.0f;
                viewport.y = static_cast<f32>(height);
                viewport.width = static_cast<f32>(width);
                viewport.height = -static_cast<f32>(height);
            }
            else
            {
                width = frame_buffer->get_width();
                height = frame_buffer->get_height();
                render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_begin_info.pNext = nullptr;
                render_pass_begin_info.renderPass = vulkan_frame_buffer->GetVkRenderPass();
                render_pass_begin_info.renderArea.offset.x = 0;
                render_pass_begin_info.renderArea.offset.y = 0;
                render_pass_begin_info.renderArea.extent.width = width;
                render_pass_begin_info.renderArea.extent.height = height;
                render_pass_begin_info.framebuffer = vulkan_frame_buffer->GetVkFramebuffer();
                KB_CORE_ASSERT(render_pass_begin_info.framebuffer, "render pass framebuffer is null?");

                viewport.x = 0.0f;
                viewport.y = 0.0f;
                viewport.width = static_cast<f32>(width);
                viewport.height = static_cast<f32>(height);
            }

            const auto& clear_values = vulkan_frame_buffer->GetVkClearValues();
            render_pass_begin_info.clearValueCount = static_cast<uint32_t>(clear_values.size());
            render_pass_begin_info.pClearValues = clear_values.data();

            vkCmdBeginRenderPass(vk_command_buffer, &render_pass_begin_info, VK_SUBPASS_CONTENTS_INLINE);

            if (p_explicit_clear)
            {
                const u32 color_attachment_count = static_cast<u32>(vulkan_frame_buffer->GetColorAttachmentCount());
                const u32 total_attachment_count = color_attachment_count +
                    (vulkan_frame_buffer->HasDepthAttachment() ? 1 : 0);
                KB_CORE_ASSERT(
                    clear_values.size() == total_attachment_count,
                    "[vulkan_renderer_backend]: Clear value size does not match attachment count!"
                );

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

                if (vulkan_frame_buffer->HasDepthAttachment())
                {
                    attachments[color_attachment_count].aspectMask =
                        VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
                    attachments[color_attachment_count].clearValue = clear_values[color_attachment_count];
                    clear_rects[color_attachment_count].rect.offset = { 0, 0 };
                    clear_rects[color_attachment_count].rect.extent = { width, height };
                    clear_rects[color_attachment_count].baseArrayLayer = 0;
                    clear_rects[color_attachment_count].layerCount = 1;
                }

                vkCmdClearAttachments(
                    vk_command_buffer,
                    total_attachment_count,
                    attachments.data(),
                    total_attachment_count,
                    clear_rects.data()
                );
            }

            // Update dynamic viewport state
            vkCmdSetViewport(vk_command_buffer, 0, 1, &viewport);

            // Update dynamic scissor state
            VkRect2D scissor;
            scissor.extent.width = width;
            scissor.extent.height = height;
            scissor.offset.x = 0;
            scissor.offset.y = 0;
            vkCmdSetScissor(vk_command_buffer, 0, 1, &scissor);

            // bind vulkan pipeline
            auto vulkan_pipeline = p_render_pass->get_specification().m_pipeline.As<VulkanPipeline>();
            const auto vk_pipeline = vulkan_pipeline->get_vk_pipeline();
            vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

            // #TODO set dynamic line width

            ref<vulkan_render_pass> render_pass = p_render_pass.As<vulkan_render_pass>();
            render_pass->rt_prepare();
            if (render_pass->has_descriptor_sets())
            {
                const auto& descriptor_sets = render_pass->get_descriptor_sets(frame_index);
                vkCmdBindDescriptorSets(
                    vk_command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    vulkan_pipeline->get_vk_pipeline_layout(),
                    render_pass->get_first_set_index(),
                    static_cast<u32>(descriptor_sets.size()),
                    descriptor_sets.data(),
                    0,
                    nullptr
                );
            }
        }
    );
}

auto vulkan_render_backend::end_render_pass(
    const ref<RenderCommandBuffer>& p_render_command_buffer
) noexcept -> void
{
    submit([p_render_command_buffer]()
        {
            KB_PROFILE_SCOPE_NAMED("vulkan_render_backend::end_render_pass");
            log::core::info(
                log::logger_tag_t::renderer,
                "vulkan_render_backend::end_render_pass"
            );
            const auto vk_command_buffer = p_render_command_buffer.As<VulkanRenderCommandBuffer>()->get_active_command_buffer();

            vkCmdEndRenderPass(vk_command_buffer);
        }
    );
}

auto vulkan_render_backend::set_line_width(const ref<RenderCommandBuffer>& render_command_buffer,
                                           f32 line_width) noexcept -> void
{
    submit([width = line_width, render_cmd_buffer = render_command_buffer]()
        {
            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_cmd_buffer = render_cmd_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);
            vkCmdSetLineWidth(vk_cmd_buffer, width);
        });
}

auto vulkan_render_backend::submit_fullscreen_quad(
    const ref<RenderCommandBuffer>& p_render_command_buffer,
    const ref<Pipeline>& p_pipeline,
    const ref<Material>& p_material
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    ref<VulkanMaterial> vulkan_material = p_material.As<VulkanMaterial>();
    render::submit([p_render_command_buffer, p_pipeline, vulkan_material, this]() mutable
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer = p_render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

            ref<VulkanPipeline> vulkan_pipeline = p_pipeline.As<VulkanPipeline>();

            const VkPipelineLayout layout = vulkan_pipeline->get_vk_pipeline_layout();

            auto vulkan_vertex_buffer = m_renderer_data->m_quad_vertex_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, offsets);

            auto vulkan_index_buffer = m_renderer_data->m_quad_index_buffer.As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = vulkan_index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            if (vulkan_material)
            {
                const VkDescriptorSet descriptor_set = vulkan_material->get_vk_descriptor_set(frame_index);
                if (descriptor_set)
                {
                    vkCmdBindDescriptorSets(
                        vk_command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        layout,
                        0,
                        1,
                        &descriptor_set,
                        0,
                        nullptr
                    );
                }

                const owning_buffer& uniform_storage_buffer = vulkan_material->get_uniform_storage_buffer();
                if (uniform_storage_buffer.size())
                {
                    vkCmdPushConstants(
                        vk_command_buffer,
                        layout,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        static_cast<u32>(uniform_storage_buffer.size()),
                        uniform_storage_buffer.get()
                    );

#ifdef KB_DEBUG
                    log::core::trace(
                        log::logger_tag_t::renderer,
                        "[vulkan_render_backend]: Push constant size {}",
                        uniform_storage_buffer.size()
                    );
#endif
                }
            }

            vkCmdDrawIndexed(
                vk_command_buffer,
                m_renderer_data->m_quad_index_buffer->GetCount(),
                1,
                0,
                0,
                0
            );
        });
}

auto vulkan_render_backend::render_geometry(
    const ref<RenderCommandBuffer>& p_render_command_buffer,
    const ref<Pipeline>& p_pipeline,
    const ref<Material>& p_material,
    const ref<VertexBuffer>& p_vertex_buffer,
    const ref<IndexBuffer>& p_index_buffer,
    const glm::mat4& p_transform,
    uint32_t p_index_count
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    ref<VulkanMaterial> vulkan_material = p_material.As<VulkanMaterial>();
    if (p_index_count == 0)
        p_index_count = p_index_buffer->GetCount();

    submit([p_render_command_buffer, p_pipeline, vulkan_material, p_vertex_buffer, p_index_buffer, p_transform, p_index_count]() mutable
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer command_buffer = p_render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

            ref<VulkanPipeline> vulkan_pipeline = p_pipeline.As<VulkanPipeline>();

            const VkPipelineLayout layout = vulkan_pipeline->get_vk_pipeline_layout();

            auto vulkan_geometry_vertex_buffer = p_vertex_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vulkan_geometry_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 }; // wtf is this
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

            auto vulkan_geometry_index_buffer = p_index_buffer.As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = vulkan_geometry_index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkDescriptorSet descriptor_set = vulkan_material->get_vk_descriptor_set(frame_index);
            if (descriptor_set)
            {
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    layout,
                    0,
                    1,
                    &descriptor_set,
                    0,
                    nullptr
                );
            }

            vkCmdPushConstants(
                command_buffer,
                layout,
                VK_SHADER_STAGE_VERTEX_BIT,
                0,
                sizeof(glm::mat4),
                &p_transform
            );

            const owning_buffer& uniform_storage_buffer = vulkan_material->get_uniform_storage_buffer();
            if (uniform_storage_buffer)
            {
                vkCmdPushConstants(
                    command_buffer,
                    layout,
                    VK_SHADER_STAGE_FRAGMENT_BIT,
                    sizeof(glm::mat4),
                    static_cast<u32>(uniform_storage_buffer.size()),
                    uniform_storage_buffer.get()
                );
            }

            vkCmdDrawIndexed(
                command_buffer,
                p_index_count,
                1,
                0,
                0,
                0
            );
        });
}

auto vulkan_render_backend::render_instanced_submesh(
    const ref<RenderCommandBuffer>& p_render_command_buffer,
    const ref<Pipeline>& p_pipeline,
    const ref<Mesh>& p_mesh,
    u32 p_index,
    const ref<MaterialTable>& p_material_table,
    const ref<VertexBuffer>& p_transform_buffer,
    u32 p_transform_offset,
    u32 p_bone_transforms_offset,
    u32 p_instance_count
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    render::submit([p_render_command_buffer, p_pipeline, p_mesh, p_index, p_material_table, p_transform_buffer, p_transform_offset, p_instance_count]()
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer =
                p_render_command_buffer.As<VulkanRenderCommandBuffer>()->GetCommandBuffer(frame_index);

            // retrieve mesh data vertex buffer and bind
            ref<MeshData> mesh_data = p_mesh->GetMeshData();
            ref<VulkanVertexBuffer> vertex_buffer = mesh_data->GetVertexBuffer().As<VulkanVertexBuffer>();
            const VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize vertex_offsets[1] = { 0 };
            vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, vertex_offsets);

            // retrieve mesh transform vertex buffer and bind
            ref<VulkanVertexBuffer> vulkan_transform_buffer = p_transform_buffer.As<VulkanVertexBuffer>();
            const VkBuffer vk_transform_buffer = vulkan_transform_buffer->GetVkBuffer();
            const VkDeviceSize transform_offsets[1] = { p_transform_offset };
            vkCmdBindVertexBuffers(vk_command_buffer, 1, 1, &vk_transform_buffer, transform_offsets);

            ref<VulkanIndexBuffer> index_buffer = mesh_data->GetIndexBuffer().As<VulkanIndexBuffer>();
            const VkBuffer vk_index_buffer = index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const auto& mesh_asset_submeshes = mesh_data->GetSubmeshes();
            const Submesh& submesh = mesh_asset_submeshes[p_index];
            const auto& mesh_material_table = p_mesh->GetMaterials();
            uint32_t material_count = mesh_material_table->GetMaterialCount();
            ref<MaterialAsset> material = p_material_table->HasMaterial(submesh.Material_index) ?
                p_material_table->GetMaterial(submesh.Material_index) :
                mesh_material_table->GetMaterial(submesh.Material_index);
            ref<VulkanMaterial> vulkan_material = material->GetMaterial().As<VulkanMaterial>();

            ref<VulkanPipeline> vulkan_pipeline = p_pipeline.As<VulkanPipeline>();
            const VkPipeline vk_pipeline = vulkan_pipeline->get_vk_pipeline();
            const VkPipelineLayout vk_pipeline_layout = vulkan_pipeline->get_vk_pipeline_layout();
            vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

            // #TODO line width

            if (vulkan_material)
            {
                const auto vk_descriptor_set = vulkan_material->get_vk_descriptor_set(frame_index);
                if (vk_descriptor_set)
                {
                    vkCmdBindDescriptorSets(
                        vk_command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        vk_pipeline_layout,
                        0,
                        1,
                        &vk_descriptor_set,
                        0,
                        nullptr
                    );
                }

                owning_buffer uniform_storage_buffer = vulkan_material->get_uniform_storage_buffer();
                if (uniform_storage_buffer)
                {
                    vkCmdPushConstants(
                        vk_command_buffer,
                        vk_pipeline_layout,
                        VK_SHADER_STAGE_FRAGMENT_BIT,
                        0,
                        static_cast<uint32_t>(uniform_storage_buffer.size()),
                        uniform_storage_buffer.get()
                    );
                }
            }

            vkCmdDrawIndexed(
                vk_command_buffer,
                submesh.IndexCount,
                p_instance_count,
                submesh.BaseIndex,
                static_cast<i32>(submesh.BaseVertex),
                0
            );
        }
    );
}

auto vulkan_render_backend::rt_allocate_descriptor_set(
    VkDescriptorSetAllocateInfo& p_alloc_info
) const noexcept -> VkDescriptorSet
{
    KB_PROFILE_SCOPE;

    const auto buffer_index = rt_get_current_frame_index();
    p_alloc_info.descriptorPool = m_renderer_data->m_descriptor_pools[buffer_index];
    const auto vk_device = vk::get_current_vk_device();
    VkDescriptorSet vk_descriptor_set;
    KB_VK_CHECK_RESULT(
        vkAllocateDescriptorSets(vk_device, &p_alloc_info, &vk_descriptor_set)
    );
    m_renderer_data->m_descriptor_pool_allocation_count[buffer_index] += p_alloc_info.descriptorSetCount;
    return vk_descriptor_set;
}

}
