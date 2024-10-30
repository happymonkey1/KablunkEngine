#include "kablunkpch.h"

#include "kablunk/renderer/backend/vulkan/vulkan_render_backend.h"

#include "Kablunk/Renderer/RenderCommand.h"

#include "kablunk/renderer/backend/vulkan/vulkan_core.h"
#include "kablunk/renderer/backend/vulkan/vulkan_api.h"
#include "kablunk/renderer/backend/vulkan/vulkan_context.h"
#include "kablunk/renderer/backend/vulkan/vulkan_frame_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_index_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_material.h"
#include "kablunk/renderer/backend/vulkan/vulkan_pipeline.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_command_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_vertex_buffer.h"
#include "kablunk/renderer/backend/vulkan/vulkan_render_pass.h"

#include <vulkan/vulkan.h>

#include "Kablunk/Core/Application.h"

namespace kb::render::backend::vk
{ // start namespace kb::render::backend::vk

struct vulkan_render_backend_data
{
    arc<VertexBuffer> m_quad_vertex_buffer{};
    arc<IndexBuffer> m_quad_index_buffer{};
    vulkan_shader::ShaderMaterialDescriptorSet m_quad_descriptor_set{};

    VkDescriptorSet m_active_descriptor_set = nullptr;
    std::vector<VkDescriptorPool> m_descriptor_pools{};
    VkDescriptorPool m_material_descriptor_pool = nullptr;
    std::vector<uint32_t> m_descriptor_pool_allocation_count{};

    // UniformBufferSet -> Shader Hash -> Frame -> WriteDescriptor
    unordered_flat_map<uniform_buffer_set*, unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> uniform_buffer_write_descriptor_cache{};
    // StorageBufferSet -> Shader Hash -> Frame -> WriteDescriptor
    unordered_flat_map<storage_buffer_set*, unordered_flat_map<uint64_t, std::vector<std::vector<VkWriteDescriptorSet>>>> storage_buffer_write_descriptor_cache{};

    int32_t draw_call_count = 0;
};

namespace
{
std::unique_ptr<vulkan_render_backend_data> s_renderer_data{};
}

auto vulkan_render_backend::init() noexcept -> void
{
    s_renderer_data = std::make_unique<vulkan_render_backend_data>();
    const auto frames_in_flight = render::get_frames_in_flight();

    s_renderer_data->m_descriptor_pools.resize(frames_in_flight);
    s_renderer_data->m_descriptor_pool_allocation_count.resize(frames_in_flight);

    submit([frames_in_flight]() mutable
        {
            constexpr size_t k_individual_pool_size = 1000ull;
            const VkDescriptorPoolSize vk_pool_sizes[] =
            {
                { VK_DESCRIPTOR_TYPE_SAMPLER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, k_individual_pool_size },
                { VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, k_individual_pool_size }
            };

            constexpr size_t k_pool_count = sizeof(vk_pool_sizes) / sizeof(VkDescriptorPoolSize);
            const VkDescriptorPoolCreateInfo vk_pool_info{
                .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
                .pNext = nullptr,
                .flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT,
                .maxSets = 2000ull,
                .poolSizeCount = k_pool_count,
                .pPoolSizes = vk_pool_sizes
            };

            // per-frame renderer descriptor pools
            const auto vk_device = get_current_vk_device();
            for (u32 i = 0; i < frames_in_flight; i++)
            {
                KB_VK_CHECK_RESULT(
                    vkCreateDescriptorPool(vk_device, &vk_pool_info, nullptr, &s_renderer_data->m_descriptor_pools[i])
                );
                s_renderer_data->m_descriptor_pool_allocation_count[i] = 0;
            }

            // material specific descriptor pool
            KB_VK_CHECK_RESULT(
                vkCreateDescriptorPool(vk_device, &vk_pool_info, nullptr, &s_renderer_data->m_material_descriptor_pool)
            );
        }
    );

    constexpr f32 x = -1.f;
    constexpr f32 y = -1.f;
    constexpr f32 width = 2.f;
    constexpr f32 height = 2.f;
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
    s_renderer_data->m_quad_vertex_buffer = VertexBuffer::Create(quad_data, 4 * sizeof(QuadVertex));
    constexpr u32 indices[6] = { 0, 1, 2, 2, 3, 0, };

    log::core::info(
        log::logger_tag_t::renderer,
        "Creating fullscreen quad index buffer"
    );
    s_renderer_data->m_quad_index_buffer = IndexBuffer::Create(indices, 6 * sizeof(uint32_t));
}

auto vulkan_render_backend::shutdown() noexcept -> void
{
    KB_PROFILE_SCOPE;
    log::core::info(
        log::logger_tag_t::renderer,
        "Shutting down Vulkan render backend"
    );
    const auto vk_device = get_current_vk_device();
    vkDeviceWaitIdle(vk_device);

    for (const auto& vk_descriptor_pool : s_renderer_data->m_descriptor_pools)
    {
        vkDestroyDescriptorPool(
            vk_device,
            vk_descriptor_pool,
            nullptr
        );
    }

    vkDestroyDescriptorPool(
        vk_device,
        s_renderer_data->m_material_descriptor_pool,
        nullptr
    );

    s_renderer_data.reset();
}

auto vulkan_render_backend::begin_frame(
    weak_arc<graphics_context> p_context
) noexcept -> void
{
    const auto vulkan_context = p_context.as<vk::vulkan_context>();
    submit([vulkan_context]()
        {
            KB_PROFILE_SCOPE_NAMED("vulkan_render_backend::begin_frame");
            const auto vk_device = vulkan_context->get_device()->get_vk_device();
            const auto* swap_chain = vulkan_context->get_swap_chain();
            const auto buffer_index = swap_chain->get_current_buffer_index();

            vkResetDescriptorPool(
                vk_device,
                s_renderer_data->m_descriptor_pools[buffer_index],
                0
            );
            memset(
                s_renderer_data->m_descriptor_pool_allocation_count.data(),
                0,
                s_renderer_data->m_descriptor_pool_allocation_count.size() * sizeof(u32)
            );
            s_renderer_data->draw_call_count = 0;
        }
    );
}

auto vulkan_render_backend::end_frame() noexcept -> void
{
}

auto vulkan_render_backend::begin_render_pass(
    weak_arc<graphics_context> p_graphics_context,
    const arc<render_command_buffer>& p_render_command_buffer,
    const arc<render_pass>& p_render_pass,
    bool p_explicit_clear
) noexcept -> void
{
    submit([graphics_context = p_graphics_context, render_command_buffer = p_render_command_buffer, render_pass = p_render_pass, p_explicit_clear]()
        {
            KB_PROFILE_SCOPE_NAMED("vulkan_render_backend::begin_render_pass");
            log::core::trace(
                log::logger_tag_t::renderer,
                "vulkan_render_backend::begin_render_pass {}",
                render_pass->get_specification().m_debug_name
            );

            const auto vulkan_context = graphics_context.as<vk::vulkan_context>();


            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer = render_command_buffer.As<vulkan_render_command_buffer>()->get_active_command_buffer();

            const auto frame_buffer = render_pass->get_target_frame_buffer();
            const auto vulkan_frame_buffer = frame_buffer.As<vk::vulkan_frame_buffer>();
            const auto& frame_buffer_spec = vulkan_frame_buffer->get_specification();

            u32 width = frame_buffer_spec.m_width;
            u32 height = frame_buffer_spec.m_height;

            VkViewport viewport{};
            viewport.minDepth = 0.0f;
            viewport.maxDepth = 1.0f;

            VkRenderPassBeginInfo render_pass_begin_info{};
            render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
            render_pass_begin_info.pNext = nullptr;
            render_pass_begin_info.renderPass = vulkan_frame_buffer->get_vk_render_pass();
            render_pass_begin_info.renderArea.offset.x = 0;
            render_pass_begin_info.renderArea.offset.y = 0;
            render_pass_begin_info.renderArea.extent.width = width;
            render_pass_begin_info.renderArea.extent.height = height;
            if (frame_buffer_spec.m_swap_chain_target)
            {
                const auto* swap_chain = vulkan_context->get_vulkan_swap_chain();
                width = swap_chain->get_width();
                height = swap_chain->get_height();
                render_pass_begin_info.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
                render_pass_begin_info.pNext = nullptr;
                render_pass_begin_info.renderPass = vulkan_frame_buffer->get_vk_render_pass();
                render_pass_begin_info.renderArea.offset.x = 0;
                render_pass_begin_info.renderArea.offset.y = 0;
                render_pass_begin_info.renderArea.extent.width = width;
                render_pass_begin_info.renderArea.extent.height = height;
                render_pass_begin_info.framebuffer = swap_chain->get_current_vk_framebuffer();
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
                render_pass_begin_info.renderPass = vulkan_frame_buffer->get_vk_render_pass();
                render_pass_begin_info.renderArea.offset.x = 0;
                render_pass_begin_info.renderArea.offset.y = 0;
                render_pass_begin_info.renderArea.extent.width = width;
                render_pass_begin_info.renderArea.extent.height = height;
                render_pass_begin_info.framebuffer = vulkan_frame_buffer->get_vk_framebuffer();
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
                const u32 color_attachment_count = vulkan_frame_buffer->get_color_attachment_count();
                const u32 total_attachment_count = color_attachment_count +
                    (vulkan_frame_buffer->has_depth_attachment() ? 1 : 0);
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

                if (vulkan_frame_buffer->has_depth_attachment())
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
            auto pipeline = render_pass->get_specification().m_pipeline.As<vulkan_pipeline>();
            const auto vk_pipeline = pipeline->get_vk_pipeline();
            vkCmdBindPipeline(vk_command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, vk_pipeline);

            // #TODO set dynamic line width

            auto vulkan_render_pass = render_pass.As<vk::vulkan_render_pass>();
            vulkan_render_pass->rt_prepare();
            if (vulkan_render_pass->has_descriptor_sets())
            {
                const auto& descriptor_sets = vulkan_render_pass->get_descriptor_sets(frame_index);
                vkCmdBindDescriptorSets(
                    vk_command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    pipeline->get_vk_pipeline_layout(),
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
    const arc<render_command_buffer>& p_render_command_buffer
) noexcept -> void
{
    submit([render_command_buffer = p_render_command_buffer]()
        {
            KB_PROFILE_SCOPE_NAMED("vulkan_render_backend::end_render_pass");
            log::core::trace(
                log::logger_tag_t::renderer,
                "vulkan_render_backend::end_render_pass"
            );
            const auto vk_command_buffer = render_command_buffer.As<vulkan_render_command_buffer>()->get_active_command_buffer();

            vkCmdEndRenderPass(vk_command_buffer);
        }
    );
}

auto vulkan_render_backend::set_line_width(
    const arc<render_command_buffer>& p_render_command_buffer,
    f32 line_width
) noexcept -> void
{
    submit([width = line_width, render_command_buffer = p_render_command_buffer]()
        {
            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_cmd_buffer = render_command_buffer.As<vulkan_render_command_buffer>()->GetCommandBuffer(frame_index);
            vkCmdSetLineWidth(vk_cmd_buffer, width);
        });
}

auto vulkan_render_backend::submit_fullscreen_quad(
    const arc<render_command_buffer>& p_render_command_buffer,
    const arc<pipeline>& p_pipeline,
    const arc<material>& p_material
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    auto vulkan_material = p_material.As<vk::vulkan_material>();
    submit([render_command_buffer = p_render_command_buffer, pipeline = p_pipeline, vulkan_material]() mutable
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer = render_command_buffer.As<vulkan_render_command_buffer>()->GetCommandBuffer(frame_index);

            arc<vulkan_pipeline> vulkan_pipeline = pipeline.As<vk::vulkan_pipeline>();

            const VkPipelineLayout layout = vulkan_pipeline->get_vk_pipeline_layout();

            auto vulkan_vertex_buffer = s_renderer_data->m_quad_vertex_buffer.As<vk::vulkan_vertex_buffer>();
            const VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, offsets);

            auto vulkan_index_buffer = s_renderer_data->m_quad_index_buffer.As<vk::vulkan_index_buffer>();
            const VkBuffer vk_index_buffer = vulkan_index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            if (vulkan_material)
            {
                const VkDescriptorSet vk_descriptor_set = vulkan_material->get_vk_descriptor_set(frame_index);
                if (vk_descriptor_set)
                {
                    vkCmdBindDescriptorSets(
                        vk_command_buffer,
                        VK_PIPELINE_BIND_POINT_GRAPHICS,
                        layout,
                        0,
                        1,
                        &vk_descriptor_set,
                        0,
                        nullptr
                    );
                }
                else
                {
#if 0
#ifdef KB_DEBUG
                    log::core::warn(
                        log::logger_tag_t::renderer,
                        "[vulkan_renderer_backend::submit_fullscreen_quad]: Descriptor set {} is null?",
                        static_cast<const void*>(vk_descriptor_set)
                    );
#endif
#endif
                    // vulkan_material->rt_prepare();
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
#if 0
#ifdef KB_DEBUG
                    log::core::trace(
                        log::logger_tag_t::renderer,
                        "[vulkan_render_backend]: Push constant size {}",
                        uniform_storage_buffer.size()
                    );
#endif
#endif
                }
            }

            vkCmdDrawIndexed(
                vk_command_buffer,
                s_renderer_data->m_quad_index_buffer->GetCount(),
                1,
                0,
                0,
                0
            );
        });
}

auto vulkan_render_backend::render_geometry(
    const arc<render_command_buffer>& p_render_command_buffer,
    const arc<pipeline>& p_pipeline,
    const arc<material>& p_material,
    const arc<VertexBuffer>& p_vertex_buffer,
    const arc<IndexBuffer>& p_index_buffer,
    const glm::mat4& p_transform,
    uint32_t p_index_count
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    arc vulkan_material = p_material.As<vk::vulkan_material>();
    if (p_index_count == 0)
        p_index_count = p_index_buffer->GetCount();

    auto vulkan_pipeline = p_pipeline.As<vk::vulkan_pipeline>();
    auto vulkan_vertex_buffer = p_vertex_buffer.As<vk::vulkan_vertex_buffer>();
    auto vulkan_index_buffer = p_index_buffer.As<vk::vulkan_index_buffer>();

    submit([render_command_buffer = p_render_command_buffer, vulkan_pipeline, vulkan_material, vulkan_vertex_buffer, vulkan_index_buffer, p_transform, p_index_count]() mutable
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer command_buffer = render_command_buffer.As<vulkan_render_command_buffer>()->GetCommandBuffer(frame_index);

            const VkPipelineLayout layout = vulkan_pipeline->get_vk_pipeline_layout();

            const VkBuffer vk_vertex_buffer = vulkan_vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize offsets[1] = { 0 };
            vkCmdBindVertexBuffers(command_buffer, 0, 1, &vk_vertex_buffer, offsets);

            const VkBuffer vk_index_buffer = vulkan_index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const VkDescriptorSet vk_descriptor_set = vulkan_material->get_vk_descriptor_set(frame_index);
            if (vk_descriptor_set)
            {
                vkCmdBindDescriptorSets(
                    command_buffer,
                    VK_PIPELINE_BIND_POINT_GRAPHICS,
                    layout,
                    0,
                    1,
                    &vk_descriptor_set,
                    0,
                    nullptr
                );
            }
            else
            {
                log::core::warn(
                    log::logger_tag_t::renderer,
                    "[vulkan_renderer_backend]: Descriptor set {} is null?",
                    static_cast<const void*>(vk_descriptor_set)
                );
            }

            // push u_Renderer mat4
            // #TODO why do we still have this?
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
    arc<render_command_buffer> p_render_command_buffer,
    arc<pipeline> p_pipeline,
    arc<Mesh> p_mesh,
    u32 p_index,
    arc<MaterialTable> p_material_table,
    arc<VertexBuffer> p_transform_buffer,
    u32 p_transform_offset,
    u32 p_bone_transforms_offset,
    u32 p_instance_count
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    submit([p_render_command_buffer, p_pipeline, p_mesh, p_index, p_material_table, p_transform_buffer, p_transform_offset, p_instance_count]()
        {
            KB_PROFILE_SCOPE;

            const u32 frame_index = rt_get_current_frame_index();
            const VkCommandBuffer vk_command_buffer =
                p_render_command_buffer.As<vulkan_render_command_buffer>()->GetCommandBuffer(frame_index);

            // retrieve mesh data vertex buffer and bind
            arc<MeshData> mesh_data = p_mesh->GetMeshData();
            arc<vulkan_vertex_buffer> vertex_buffer = mesh_data->GetVertexBuffer().As<vulkan_vertex_buffer>();
            const VkBuffer vk_vertex_buffer = vertex_buffer->GetVkBuffer();
            constexpr VkDeviceSize vertex_offsets[1] = { 0 };
            vkCmdBindVertexBuffers(vk_command_buffer, 0, 1, &vk_vertex_buffer, vertex_offsets);

            // retrieve mesh transform vertex buffer and bind
            arc<vulkan_vertex_buffer> vulkan_transform_buffer = p_transform_buffer.As<vulkan_vertex_buffer>();
            const VkBuffer vk_transform_buffer = vulkan_transform_buffer->GetVkBuffer();
            const VkDeviceSize transform_offsets[1] = { p_transform_offset };
            vkCmdBindVertexBuffers(vk_command_buffer, 1, 1, &vk_transform_buffer, transform_offsets);

            arc<vulkan_index_buffer> index_buffer = mesh_data->GetIndexBuffer().As<vulkan_index_buffer>();
            const VkBuffer vk_index_buffer = index_buffer->GetVkBuffer();
            vkCmdBindIndexBuffer(vk_command_buffer, vk_index_buffer, 0, VK_INDEX_TYPE_UINT32);

            const auto& mesh_asset_submeshes = mesh_data->GetSubmeshes();
            const Submesh& submesh = mesh_asset_submeshes[p_index];
            const auto& mesh_material_table = p_mesh->GetMaterials();
            uint32_t material_count = mesh_material_table->GetMaterialCount();
            arc<MaterialAsset> material = p_material_table->HasMaterial(submesh.Material_index) ?
                p_material_table->GetMaterial(submesh.Material_index) :
                mesh_material_table->GetMaterial(submesh.Material_index);
            arc<vulkan_material> vulkan_material = material->GetMaterial().As<vk::vulkan_material>();

            arc<vulkan_pipeline> pipeline = p_pipeline.As<vk::vulkan_pipeline>();
            const VkPipeline vk_pipeline = pipeline->get_vk_pipeline();
            const VkPipelineLayout vk_pipeline_layout = pipeline->get_vk_pipeline_layout();
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
                else
                {
                    log::core::warn(
                        log::logger_tag_t::renderer,
                        "[vulkan_renderer_backend]: Descriptor set {} is null?",
                        static_cast<const void*>(vk_descriptor_set)
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

auto vulkan_render_backend::copy_image(
    arc<render_command_buffer> p_render_command_buffer,
    arc<image_2d> p_source_image,
    arc<image_2d> p_destination_image
) noexcept -> void
{
    KB_PROFILE_SCOPE;

    submit([p_render_command_buffer,
        source_image = p_source_image.As<vulkan_image_2d>(),
        destination_image = p_destination_image.As<vulkan_image_2d>()
    ]() mutable
        {
            KB_PROFILE_SCOPE_NAMED("rt_copy_image");

            const auto frame_index = rt_get_current_frame_index();
            const auto vk_command_buffer = p_render_command_buffer
                .As<vulkan_render_command_buffer>()->GetCommandBuffer(frame_index);

            auto vk_src_image = source_image->get_vk_image_info().image;
            auto vk_dst_image = destination_image->get_vk_image_info().image;

            const auto src_size = source_image->get_size();
            const auto dst_size = destination_image->get_size();
            constexpr VkImageSubresourceLayers vk_subresource_layers{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .mipLevel = 0,
                .baseArrayLayer = 0,
                .layerCount = 1
            };
            // #TODO: why do we not have a dest image copy?
            const VkImageCopy vk_image_copy{
                .srcSubresource = vk_subresource_layers,
                .srcOffset = { 0, 0, 0 },
                .dstSubresource = vk_subresource_layers,
                .dstOffset = { 0, 0, 0 },
                .extent = { src_size.x, src_size.y, 1 }
            };

            const auto vk_src_image_layout = source_image->get_vk_image_info_descriptor().imageLayout;
            const auto vk_dst_image_layout = destination_image->get_vk_image_info_descriptor().imageLayout;

            constexpr VkImageSubresourceRange vk_image_subresource_range{
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            };

            // destination image barrier
            {
                const VkImageMemoryBarrier vk_image_memory_barrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = vk_dst_image_layout,
                    .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .srcQueueFamilyIndex = {}, // #TODO: is this correct?
                    .dstQueueFamilyIndex = {}, // #TODO: is this correct?
                    .image = vk_dst_image,
                    .subresourceRange = vk_image_subresource_range
                };

                vkCmdPipelineBarrier(
                    vk_command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &vk_image_memory_barrier
                );
            }

            // source image barrier
            {
                const VkImageMemoryBarrier vk_image_memory_barrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .newLayout = vk_src_image_layout,
                    .srcQueueFamilyIndex = {},
                    .dstQueueFamilyIndex = {},
                    .image = vk_src_image,
                    .subresourceRange = vk_image_subresource_range
                };

                vkCmdPipelineBarrier(
                    vk_command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &vk_image_memory_barrier
                );
            }

            vkCmdCopyImage(
                vk_command_buffer,
                vk_src_image,
                VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                vk_dst_image,
                VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                1,
                &vk_image_copy
            );

            // source image barrier
            {
                const VkImageMemoryBarrier vk_image_memory_barrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
                    .newLayout = vk_src_image_layout,
                    .srcQueueFamilyIndex = {},
                    .dstQueueFamilyIndex = {},
                    .image = vk_src_image,
                    .subresourceRange = vk_image_subresource_range
                };

                vkCmdPipelineBarrier(
                    vk_command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &vk_image_memory_barrier
                );
            }

            // destination image barrier
            {
                const VkImageMemoryBarrier vk_image_memory_barrier{
                    .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
                    .pNext = nullptr,
                    .srcAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
                    .dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT,
                    .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                    .newLayout = vk_dst_image_layout,
                    .srcQueueFamilyIndex = {},
                    .dstQueueFamilyIndex = {},
                    .image = vk_dst_image,
                    .subresourceRange = vk_image_subresource_range
                };

                vkCmdPipelineBarrier(
                    vk_command_buffer,
                    VK_PIPELINE_STAGE_TRANSFER_BIT,
                    VK_PIPELINE_STAGE_ALL_COMMANDS_BIT,
                    0,
                    0,
                    nullptr,
                    0,
                    nullptr,
                    1,
                    &vk_image_memory_barrier
                );
            }
        });
}

auto vulkan_render_backend::rt_allocate_descriptor_set(
    VkDescriptorSetAllocateInfo& p_alloc_info
) noexcept -> VkDescriptorSet
{
    KB_PROFILE_SCOPE;

    const auto buffer_index = rt_get_current_frame_index();
    p_alloc_info.descriptorPool = s_renderer_data->m_descriptor_pools[buffer_index];
    const auto vk_device = vk::get_current_vk_device();
    VkDescriptorSet vk_descriptor_set;
    const auto res = vkAllocateDescriptorSets(vk_device, &p_alloc_info, &vk_descriptor_set);
    if (res != VK_SUCCESS)
    {
        switch (res)
        {
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_POOL_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_HOST_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_DEVICE_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_FRAGMENTED_POOL:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_FRAGMENTED_POOL!"
            );
            break;
        }
        default:
            break;
        }

        KB_CORE_ASSERT(false, "[vulkan_render_backend]: rt_allocate_descriptor_set() failed!");
    }

    s_renderer_data->m_descriptor_pool_allocation_count[buffer_index] += p_alloc_info.descriptorSetCount;
    return vk_descriptor_set;
}

auto vulkan_render_backend::rt_allocate_material_descriptor_set(
    weak_arc<graphics_context> p_context,
    VkDescriptorSetAllocateInfo& p_alloc_info
) noexcept -> VkDescriptorSet
{
    KB_PROFILE_SCOPE;

    p_alloc_info.descriptorPool = s_renderer_data->m_material_descriptor_pool;
    const auto vk_device = p_context.as<vulkan_context>()->get_device()->get_vk_device();
    VkDescriptorSet vk_descriptor_set;
    const auto res = vkAllocateDescriptorSets(vk_device, &p_alloc_info, &vk_descriptor_set);
    if (res != VK_SUCCESS)
    {
        switch (res)
        {
        case VK_ERROR_OUT_OF_POOL_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_POOL_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_OUT_OF_HOST_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_HOST_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_OUT_OF_DEVICE_MEMORY:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_OUT_OF_DEVICE_MEMORY! There is probably a bad memory leak..."
            );
            break;
        }
        case VK_ERROR_FRAGMENTED_POOL:
        {
            kb::log::core::error(
                log::logger_tag_t::renderer,
                "[vulkan_render_backend]: rt_allocate_descriptor_set failed with VK_ERROR_FRAGMENTED_POOL!"
            );
            break;
        }
        default:
            break;
        }

        KB_CORE_ASSERT(false, "[vulkan_render_backend]: rt_allocate_descriptor_set() failed!");
    }
    return vk_descriptor_set;
}
} // end namespace kb::render::backend::vk
