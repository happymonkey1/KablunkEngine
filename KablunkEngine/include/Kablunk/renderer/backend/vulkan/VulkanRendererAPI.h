#pragma once
#ifndef KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_API_H
#define KABLUNK_RENDERER_BACKEND_VULKAN_RENDER_API_H

#include "Kablunk/Core/RefCounting.h"
#include "Kablunk/Renderer/RendererAPI.h"

#include "kablunk/renderer/backend/vulkan/vulkan_render_pass.h"
#include "kablunk/renderer/backend/vulkan/vulkan_material.h"
#include "Kablunk/Renderer/MaterialAsset.h"

#include <vulkan/vulkan.h>

#if 0 

namespace kb
{ // start namespace kb
class VulkanRendererAPI final : public RendererAPI
{
public:
	VulkanRendererAPI() {}
	virtual ~VulkanRendererAPI() override;

	virtual void Init() override;
	virtual void Shutdown() override;

	virtual void BeginFrame() override;
	virtual void EndFrame() override;

	virtual void SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height) override;

	virtual void SetClearColor(const glm::vec4& color) override;
	virtual void Clear() override;

	virtual void DrawIndexed(const arc<VertexArray>& vertexArray, uint32_t indexCount = 0) override;

	virtual void ClearImage(arc<RenderCommandBuffer> commandBuffer, arc<Image2D> image) override;

	virtual void SetWireframeMode(bool draw_wireframe) override { m_draw_wireframe = draw_wireframe; }

	// ======
	// Meshes
	// ======


	virtual void RenderMesh(arc<RenderCommandBuffer> render_command_buffer, arc<render::Pipeline> pipeline, arc<UniformBufferSet> uniform_buffer_set, arc<StorageBufferSet> storage_buffer_set, arc<Mesh> mesh, uint32_t submesh_index, arc<MaterialTable> material_table, arc<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count) override;

	virtual void RenderMeshWithMaterial(arc<RenderCommandBuffer> render_command_buffer, arc<render::Pipeline> pipeline, arc<UniformBufferSet> uniform_buffer_set, arc<StorageBufferSet> storage_buffer_set, arc<Mesh> mesh, uint32_t submesh_index, arc<Material> material, arc<VertexBuffer> transform_buffer, uint32_t transform_offset, uint32_t instance_count, owning_buffer additional_uniforms) override;

	// #TODO submeshes
	virtual void render_instanced_submesh(
		arc<RenderCommandBuffer> render_command_buffer,
		arc<render::Pipeline> pipeline,
		arc<UniformBufferSet> uniform_buffer_set,
		arc<StorageBufferSet> storage_buffer_set,
		arc<Mesh> mesh,
		uint32_t submesh_index,
		arc<MaterialTable> material_table,
		arc<VertexBuffer> transform_buffer,
		uint32_t transform_offset,
		uint32_t instance_count
	) override;
	// #TODO instanced rendering

	virtual void SubmitFullscreenQuad(arc<RenderCommandBuffer> render_command_buffer, arc<render::Pipeline> pipeline, arc<UniformBufferSet> uniform_buffer_set, arc<StorageBufferSet> storage_buffer_set, arc<Material> material);

	virtual void RenderQuad(arc<RenderCommandBuffer> render_command_buffer, arc<render::Pipeline> pipeline, arc<UniformBufferSet> uniform_buffer_set, arc<StorageBufferSet> storage_buffer_set, arc<Material> material, const glm::mat4& transform) override;

	virtual void RenderGeometry(
        arc<RenderCommandBuffer> render_command_buffer,
        arc<render::Pipeline> pipeline,
        arc<UniformBufferSet> uniform_buffer_set,
        arc<StorageBufferSet> storage_buffer_set,
        arc<Material> material,
        arc<VertexBuffer> vertex_buffer,
        arc<IndexBuffer> index_buffer,
        const glm::mat4& transform,
        uint32_t index_count = 0
    ) override;


	virtual void SetLineWidth(arc<RenderCommandBuffer> render_command_buffer, float line_width) override;

	virtual void BeginRenderPass(arc<RenderCommandBuffer> render_command_buffer, const arc<render::render_pass>& render_pass, bool explicit_clear = false) override;
	virtual void EndRenderPass(arc<RenderCommandBuffer> render_command_buffer) override;

	virtual void WaitAndRender() override;

	// Vulkan Only
	static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateUniformBufferWriteDescriptors(arc<UniformBufferSet> uniform_buffer_set, arc<VulkanMaterial> material);
	static const std::vector<std::vector<VkWriteDescriptorSet>>& RT_RetrieveOrCreateStorageBufferWriteDescriptors(arc<StorageBufferSet> storage_buffer_set, arc<VulkanMaterial> material);
	static void RT_UpdateMaterialForRendering(arc<VulkanMaterial> vulkan_material, arc<UniformBufferSet> uniform_buffer_set, arc<StorageBufferSet> storage_buffer_set);
	static VkDescriptorSet RT_AllocateDescriptorSet(VkDescriptorSetAllocateInfo& alloc_info);

    [[nodiscard]] static auto rt_allocate_material_descriptor_set(VkDescriptorSetAllocateInfo& p_alloc_info) noexcept -> VkDescriptorSet;
private:
	bool m_draw_wireframe{ false };
};
} // end namespace kb

#endif

#endif
